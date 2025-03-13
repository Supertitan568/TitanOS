#include "isr.h"
#include "vga.h"
#include "acpi.h"
#include "apic.h"
#include <stdint.h>

#ifdef TEST
 #include "unit_tests.h"
#endif 

static void apic_setup(){
  void* local_apic_regs = (void*) 0x400000;
  local_apic_enable(local_apic_regs); 
  
  remap_ioredtbl((void*) 0x401000); 
}

int kernel_start(){
  disable_8259(); 
  setup_idt();
  load_idt(); 
  clear_console();
  
  // if(rsdp_ptr){
  //   printstr("RSDP at ");
  //   printlong((uint64_t) rsdp_ptr);
  //   printc('\n');
  //   
  //   printstr("RSDP Signature: ");
  //   for(int i = 0; i < 8; i++){
  //     printc(rsdp_ptr->signature[i])
  //   }
  //   printc('\n');
  //
  //   printstr("RSDP Version: ");
  //   printc(rsdp_ptr->revision + 0x30);
  //   printc('\n');
  //   
  //   printstr("OEM ID: ");
  //   for(int i = 0; i < 6; i++){
  //     printc(rsdp_ptr->oemid[i]);
  //   }
  //   printc('\n');
  //   
  //   if(validate_rsdp(rsdp_ptr)){
  //     printstr("RSDP validated\n");
  //   }
  //   else{
  //     printstr("RSDP validation failed :(\nRSDP Size:");
  //     printlong((uint64_t) sizeof(struct rsdp_descriptor_t));
  //     printc('\n');
  //   }
  // }

   
  // printstr("APIC Table Addr: ");
  // printlong((uint64_t) apic_ptr);
  // printc('\n');
      
  // TODO: Page this out here when we have the page allocator
  apic_setup();

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
