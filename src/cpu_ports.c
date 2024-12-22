#include "cpu_ports.h"

uint8_t inb(uint16_t port){
  uint8_t result;
  asm volatile("in %%dx, %%al" : "=a" (result): "d" (port));
  return result;
}

void outb(uint16_t port, uint8_t byte){
  asm volatile("out %%al, %%dx" : : "a" (byte), "d" (port));
}
  
