#include "kernel_heap_manager.h"
#include "isr.h"
#include "vga.h"
#include "apic.h"
#include "scheduler.h"
#include "keyboard.h"
#include "vmm.h"
#include <stdint.h>

#ifdef TEST
 #include "unit_tests.h"
#endif 

// This is a copy of the gdt used in the bootloader
extern char gdt64_desc[];

// This will be put at the end of the kernel by the linker
extern char kernel_end;

int kernel_start();

static void remap_gdt(){
  void* gdt_desc = (void*) gdt64_desc; 

  asm("lgdt (%0)" : "=r"(gdt_desc)); 
}

static inline uint32_t calculate_kernel_size(){
  return (((uintptr_t) kernel_start) - ((uintptr_t) &kernel_end));
}

// Bruh this was literally the only way I got this stupid thing to be at the beginning
__attribute__ ((section (".text.begin"))) 
int kernel_start(){
  // Gotta setup the idt so it doesn't triple fault on setup
  disable_8259();
  remap_gdt();
   
  setup_idt();
  load_idt();
  vmm_init();
  // Setting up console
  console_init();

  apic_setup();
  // check_scanset(); 
  printf("Kernel End: %lu\n", (uint64_t) &kernel_end);
  
  sched_init();

  
  #if defined(TEST) 
   run_unit_tests();
  #else
    printf("The unit tests were not run...\n");
  #endif

  while(1){
    continue;
  }
  return 0;
}      
