#include "kernel_heap_manager.h"
#include "isr.h"
#include "pmm.h"
#include "vga.h"
#include "apic.h"
#include "scheduler.h"
#include "keyboard.h"
#include "gdt64.h"
#include <stdint.h>
#include <mb2_mmap.h>
#include <acpi.h>

#ifdef TEST
 #include "unit_tests.h"
#endif 

// This is a copy of the gdt used in the bootloader
extern char gdt64_desc[];

// This will be put at the end of the kernel by the linker
extern char kernel_end;

static void print_name(){
  printstr("  _______ __              ____  _____\n"
           " /_  __(_) /_____ _____  / __ \\/ ___/\n"
           "  / / / / __/ __ `/ __ \\/ / / /\\__ \\ \n" 
           " / / / / /_/ /_/ / / / / /_/ /___/ / \n"
           "/_/ /_/\\__/\\__,_/_/ /_/\\____//____/  \n\n");
}

void hang(){
  while(1){
    continue;
  }
}

__attribute__ ((section (".text.begin"))) 
int kernel_start(struct multiboot_info* mb_info){
  // Diabling legacy IC 
  disable_8259();

  // Setting up console
  console_init();
  print_name();

  // Gotta setup the idt so it doesn't triple fault on setup
  setup_idt();
  load_idt();

  // Initalizing mm subsystem
  mb2_mmap_init(mb_info);
  kheap_init();
  vmm_t inital_vmm = vmm_init();
  pmm_init();

  // Remapping gdt so we can add the tss  
  remap_gdt();
  
  acpi_init(mb_info);
  // Setting up the apic for the keyboard and other things
  apic_init();
  
  // check_scanset(); 
  sched_init(inital_vmm);
  
  #if defined(TEST) 
   run_unit_tests();
  #else
    printf("The unit tests were not run...\n");
  #endif

  hang(); 
  return 0;
}


