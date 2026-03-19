#ifndef LIBTITAN_H
#define LIBTITAN_H
#include "vga.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
extern char KERNEL_OFFSET[];
extern char kernel_byte_size[];
extern char KERNEL_PHYS[];
#define LOG(subsystem, message) printf("\n%s: %s\n", subsystem, message);
#define ERROR(subsystem, message) printf("\n%s error: %s\n", subsystem, message);
#define PANIC(subsystem, message) \
  printf("\nKernel Panic in %s: %s\n"); \
  while(1){ \
    continue; \
  }
#define ASSERT(condition) \
  if(condition)\
    PANIC("ASSERT", "Failed ASSERT")
#define ALIGN_UP(x, a) \
({ typeof(x) _x = (x); \
   typeof(a) _a = (a); \
   (_x + (_a - 1)) & ~(_a - 1); })

#define PAGE_ALIGN(n) (((n) + 0xfff) / 0x1000) * 0x1000
#define PAGE_ALIGN_DOWN(n) (n & 0xfffffffffffff000)
// Basically just calculates the virtual address from a physical address when it is not page aligned 
#define CALC_VIRT_ADDRESS(p,v) (PAGE_ALIGN_DOWN(v) + (p & 0xfff))
#define PAGE_SIZE 0x1000

static inline int pow(int x, int y){
  // TODO: Implement this in assembly
  if(y == 0) {return 1;}
  for(int i = 1; i < y; i++){x *= x;}

  return x;
}

static inline uintptr_t phys_to_virt(uintptr_t ptr){
  // Basically just does the opposite of what I did in the linker script
  return ptr + ((uintptr_t)KERNEL_OFFSET);
}

static inline uintptr_t get_kernel_phys_end(){
  return (uintptr_t) KERNEL_PHYS + (uintptr_t) kernel_byte_size; 
}

#endif // !LIBTITAN_H
