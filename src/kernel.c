#include "vga.h"

int kernel_start(){
  console_init();
  printc('c');
  while(1){
    continue;
  }
  return 0;
 }      
