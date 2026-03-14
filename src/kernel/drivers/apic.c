#include "apic.h"
#include "acpi.h"
#include "cpu_ports.h"
#include "vga.h"
#include "vmm.h"
#include <stdbool.h>
#include <stdint.h>

#define PIC_MASTER_COMMAND_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21

#define PIC_SLAVE_COMMAND_PORT 0xa0
#define PIC_SLAVE_DATA_PORT 0xa1

void* local_apic_regs;
void* io_apic_regs;

void disable_8259(){
  // ICW that starts initalization
  outb(PIC_MASTER_COMMAND_PORT, 0x11);
  outb(PIC_SLAVE_COMMAND_PORT, 0x11);
  
  // Selecting the first 32 IRQs
  outb(PIC_MASTER_DATA_PORT, 0x20);
  outb(PIC_SLAVE_DATA_PORT, 0x28);
  
  // Selecting Slave Pin
  outb(PIC_MASTER_DATA_PORT, 0x2);
  outb(PIC_SLAVE_DATA_PORT, 0x4);

  // Selecting 8086 mode
  outb(PIC_MASTER_DATA_PORT, 0x1);
  outb(PIC_SLAVE_DATA_PORT, 0x1);

  // Masking all Interrupts
  outb(PIC_MASTER_DATA_PORT, 0xff);
  outb(PIC_SLAVE_DATA_PORT, 0xff);
}

void apic_setup(){
  local_apic_regs = vmm_alloc((uintptr_t) 0xffffff8000400000, 0x1000, VM_FLAG_MMIO | 0x10, (void*) 0xfee00000);
  local_apic_enable(local_apic_regs); 
  
  io_apic_regs = vmm_alloc((uintptr_t) 0xffffff8000401000, 0x1000, VM_FLAG_MMIO | 0x10, (void*) 0xfec00000);
  
  remap_ioredtbl((void*) io_apic_regs); 
}

void* get_local_apic_addr(){
  uint64_t msr_reg; 
  asm volatile ("mov $0x1b, %%rcx;"
                "rdmsr;" 
                "mov %%rax, %0;" : "=a" (msr_reg) :);
  printlong((uint64_t) msr_reg);
  printc('\n');
  // Returning the base addr of local apic in bits 12 to 51
  return (void*) ((msr_reg >> 12) & 0x000fffff); 
}



void local_apic_enable(void* local_apic_regs){
  volatile uint32_t* spurious_vec_reg_ptr = ((uint32_t*) (((uint8_t*) local_apic_regs) + 0xf0));
  
  // Enabling Local APIC and setting cpu vec to 0xf0
  *spurious_vec_reg_ptr &= 0xffffff00;
  *spurious_vec_reg_ptr |= 0x000001f0;
}

void send_local_apic_eoi(void* local_apic_regs){
  *((volatile uint32_t*) (((uintptr_t) local_apic_regs) + 0xb0)) = 0;
}

void* get_io_apic_ptr(){
  // Need Access to the MADT table
  struct rsdp_descriptor_t* rsdp_ptr = get_rsdp();
  struct rsdp_t* apic_ptr = get_acpi_table(rsdp_ptr, "APIC");

  // Entries start at 0x2c
  uint8_t* madt_entries = (void*) apic_ptr;
  madt_entries += 0x2c;

  // Goes through the MADT table until it finds a type 1 entry
  while(1){
    if(*madt_entries != 1){
      madt_entries += *(madt_entries + 1);
    }
    else{
      return (void*) (*((uint32_t*) (madt_entries + 4)));
    }
  }

  return 0x0;
}


void write_io_apic_reg(void* io_apic_base, uint8_t offset, uint32_t val){
  *((volatile uint32_t*) io_apic_base) = offset;
  *((volatile uint32_t*) (((uint8_t*)io_apic_base) + 0x10)) = val;
}


uint32_t read_io_apic_reg(void* io_apic_base, uint8_t offset){
  *((volatile uint32_t*) io_apic_base) = offset;
  return *((volatile uint32_t*) (((uint8_t*)io_apic_base) + 0x10)); 
}


void remap_ioredtbl(void* io_apic_base){
  // Writing the first 32 bits of the ioredtbl entry
  uint32_t ioredtbl_entry = read_io_apic_reg(io_apic_base, IOREDTBL(1));

  // Unmasking the irq and setting the local vector to 0x40
  ioredtbl_entry &= 0xfffeff00;
  ioredtbl_entry |= 0x00000040;
  write_io_apic_reg(io_apic_base, IOREDTBL(1), ioredtbl_entry);

  // Writing the second 32 bits of the ioredtbl entry

  ioredtbl_entry = read_io_apic_reg(io_apic_base, IOREDTBL(1) + 1);

  ioredtbl_entry &= 0xff000000;
  
  write_io_apic_reg(io_apic_base, IOREDTBL(1) + 1, ioredtbl_entry);
 
}


