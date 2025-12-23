#include <stddef.h>
#include <stdint.h>
#include "vga.h"
#include "apic.h"
#include "keyboard.h"
#include "isr.h"
#include "scheduler.h"

// TODO: Calculate this value from isr_stubs.s
#define NUM_STUBS 256
// Function Pointer to the different isr stubs
extern char start_isr_stubs[];
extern void* local_apic_regs;
struct __attribute__((packed)) idt_entry {
  uint16_t low_addr;        // 15:0 of 64 bit handler addr
  uint16_t code_selector;   // Code selector (deprecated)
  uint8_t ist;              // For switching stacks on certain interupts
  uint8_t flags;            // Different flags. Read AMD manual
  uint16_t mid_addr;        // 31:16 of 64 bit handler addr
  uint32_t high_addr;       // 63:32 of 64 bit handler addr
  uint32_t reserved;        // For the processor
};

struct idt_entry idt[256];

struct __attribute__ ((packed)) idtr{
  uint16_t limit;
  uint64_t base;
}  idt_ptr;

const char* exception_types[] = {
  "Divide-by-Zero-Error",
  "Debug",
  "NMI",
  "Breakpoint",
  "Overflow",
  "Bound-Range",
  "Invalid-Opcode",
  "Device-Not-Available",
  "Double-Fault",
  "Coprocessor-Segment-Overrun",
  "Invalid-TSS",
  "Segment-Not-Present",
  "Stack",
  "General-Protection",
  "Page-Fault"
};

static void set_idt_entry(uint8_t vec, uint64_t isr_addr, uint8_t dpl){
  struct idt_entry* entry = &idt[vec];
  
  // Packing the different parts of the isr addr into the entry
  entry->low_addr = isr_addr & 0xffff;
  entry->mid_addr = (isr_addr >> 16) & 0xffff;
  entry->high_addr = (isr_addr >> 32);
  
  // First entry in GDT
  entry->code_selector = 0x8;

  entry->flags = 0b10001110 | (dpl << 5);

  entry->ist = 0;
  entry->reserved = 0;
}

void setup_idt(){
  for(size_t vec = 0; vec < NUM_STUBS; vec++){
    set_idt_entry(vec, (uint64_t)(start_isr_stubs + (vec * 16)), 0);
  }
}

void load_idt(){
  idt_ptr.limit = NUM_STUBS * sizeof(struct idt_entry) - 1;
  idt_ptr.base = (uint64_t) idt;
  
  asm volatile("lidt (%0)" :: "r"(&idt_ptr));
  asm volatile("sti");
}

struct cpu_context_t* interrupt_handler(struct cpu_context_t* status){
  if(status->vec <= 32){
    printstr("\nKernel Panic!!: ");
    printstr(exception_types[status->vec]);
    while(1){
      continue;
    } 
  }
  else if(status->vec == 0xf0){
    printstr("Local APIC interrupt served!!\n");
  }
  else if(status->vec == 0x28){
    keyboard_handler();
    *status = schedule(status);
  }
  else{
    printstr("\nUnknown Interrupt");
    printlong((uint64_t) status->vec); 
  }
  send_local_apic_eoi(local_apic_regs);  
   
  return status;
}
