#include "isr.h"
#include "vga.h"

#ifdef TEST
 #include "unit_tests.h"
#endif 

int kernel_start(){
  setup_idt();
  load_idt(); 
  clear_console();
  
  #if defined(TEST) 
   run_unit_tests();
  #else
    const char* teststr = "The unit tests were not run...";
    printstr(teststr);
  #endif
  
  while(1){
    continue;
  }
  return 0;
}      
