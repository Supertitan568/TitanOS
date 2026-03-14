#ifndef CPU_PORTS_H
#include <stdint.h>
static inline uint8_t inb(uint16_t port){
  uint8_t result;
  asm volatile("in %%dx, %%al" : "=a" (result): "d" (port));
  return result;
}

static inline void outb(uint16_t port, uint8_t byte){
  asm volatile("out %%al, %%dx" : : "a" (byte), "d" (port));
}
#endif // !CPU_PORTS_H
