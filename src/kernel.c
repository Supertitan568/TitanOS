#include "vga.h"

int kernel_start(){
  console_init();
  const char* test_str = "Welcome to titanOS";
  clear_console();
  printstr(test_str);
  
  while(1){
    continue;
  }
  return 0;
 }      
