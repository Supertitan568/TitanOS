#include "kernel_heap_manager.h"
#include "isr.h"
#include "vga.h"
#include "apic.h"
#include "acpi.h"
#include "keyboard.h"
#include "vmm.h"
#include <stdint.h>

#ifdef TEST
 #include "unit_tests.h"
#endif 

// This will be put at the end of the kernel by the linker
extern char kernel_end;
int kernel_start();


static inline uint32_t calculate_kernel_size(){
  return (((uintptr_t) kernel_start) - ((uintptr_t) &kernel_end));
}

// Bruh this was literally the only way I got this stupid thing to be at the beginning
__attribute__ ((section (".text.begin"))) 
int kernel_start(){
  // Gotta setup the idt so it doesn't triple fault on setup
  disable_8259(); 
  setup_idt();
  load_idt(); 
  vmm_init();
  // Setting up console
  console_init();

  // TODO: Page this out here when we have the page allocator
  apic_setup();
  // check_scanset(); 

  #if defined(TEST) 
   run_unit_tests();
  #else
    printstr("The unit tests were not run...");
    printc('\n');
  #endif

  while(1){
    continue;
  }
  return 0;
}      
