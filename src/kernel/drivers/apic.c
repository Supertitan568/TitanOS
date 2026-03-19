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

typedef enum madt_types_t{
  MADT_LAPIC,
  MADT_IOAPIC,
  MADT_IOAPIC_OVERRIDE,
  MADT_IOAPIC_NMI,
  MADT_LAPIC_NMI,
  MADT_LAPIC_ADDR_OVERRIDE,
  MADT_LX2APIC
} madt_types_t;
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



void* get_lapic(){
  uintptr_t msr_reg; 
  asm volatile ("mov $0x1b, %%rcx;"
                "rdmsr;" 
                "mov %%rax, %0;" : "=a" (msr_reg) :);
  printf("apic: lapic is at %lu\n", (uint64_t) msr_reg); 
  // Returning the base addr of local apic in bits 12 to 51
  return (void*) (((msr_reg >> 12) & 0x000fffff) << 12); 
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

static void* get_ioapic(){
  //TODO: Handle case of there being more than one ioapic
  //TODO: Clean this up using structs for the madt and the entries
  // Need Access to the MADT table
  acpi_sdt_header_t* madt = get_acpi_table("APIC");
  if(madt == NULL){
    PANIC("apic", "madt could not be found")
  }
  // Entries start at after the header + 8 bytes of lapic addr and flags
  uint8_t* madt_entries = ((uint8_t*) (madt + 1)) + 8;

  // Goes through the MADT table until it finds a type 1 entry
  while((uintptr_t)madt_entries <= ((uintptr_t)madt) + madt->length){
    switch((madt_types_t) *madt_entries){
      case MADT_LAPIC:
        LOG("apic", "found an lapic");
        break;
      case MADT_IOAPIC:
        void* ioapic_phys = (void*) (*(uintptr_t*)(madt_entries + 4));
        printf("apic: ioapic is at %lu\n", (uint64_t) ioapic_phys); 
        return (void*) ioapic_phys;
      case MADT_IOAPIC_OVERRIDE:
        LOG("apic", "found an ioapic override");
        break;
      case MADT_IOAPIC_NMI:
        LOG("apic", "found an ioapic override");
        break;
      case MADT_LAPIC_NMI:
        LOG("apic", "found an lapic");
        break;
      case MADT_LAPIC_ADDR_OVERRIDE:
        LOG("apic", "found an lapic");
        break;
      default: 
        PANIC("apic", "found an unknown madt type");
    };
    madt_entries += *(madt_entries + 1);
  }
  // if we got to the end we did not find the ioapic
  return NULL;
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

void apic_init(){
  uintptr_t lapic_phys = (uintptr_t) get_lapic();
  local_apic_regs = (void*) CALC_VIRT_ADDRESS(lapic_phys, (uintptr_t)alloc_mmio((void*)PAGE_ALIGN_DOWN(lapic_phys), PAGE_SIZE, VM_FLAG_READ_WRITE));
  local_apic_enable(local_apic_regs); 
  
  void* ioapic_phys = get_ioapic();
  if(ioapic_phys == NULL){
    PANIC("apic", "ioapic not found");
  } 
  io_apic_regs = alloc_mmio(ioapic_phys, PAGE_SIZE, VM_FLAG_READ_WRITE);
  
  remap_ioredtbl(io_apic_regs); 
}
