#include "vmm.h"
#include "gdt64.h"

tss_t tss = {
  .res0 = 0,
  .rsp0 = 0,
  .rsp1 = 0,
  .rsp2 = 0,
  .res1 = 0,
  .ist1 = 0,
  .ist2 = 0,
  .ist3 = 0,
  .ist4 = 0,
  .ist5 = 0,
  .ist6 = 0,
  .ist7 = 0,
  .res2 = 0,
  .res3 = 0,
  .iopb = 0
};

gdt64_t gdt64 = {
  // Null Entry
  {},
  // Code Entry
  {
    .limit = 0xffff,
    .base0 = 0x0000,
    .base1 = 0x0,
    .access_byte =  0b10011010,
    .flags_and_limit = 0b10101111,
    .base2= 0
  },

  // Data Entry
  {
    .limit = 0xffff,
    .base0 = 0x0000,
    .base1 = 0x0,
    .access_byte =  0b10010010,
    .flags_and_limit = 0b10100000,
    .base2= 0
  },
  // User Code Entry
  {
    .limit = 0xffff,
    .base0 = 0x0000,
    .base1 = 0x0,
    .access_byte =  0b11111010,
    .flags_and_limit = 0b10101111,
    .base2= 0
  },

  // User Data Entry
  {
    .limit = 0xffff,
    .base0 = 0x0000,
    .base1 = 0x0,
    .access_byte =  0b11110010,
    .flags_and_limit = 0b10100000,
    .base2= 0
  },
  // TSS Entry
  {
    .limit = (sizeof(tss_t) - 1),
    
    /*
     * Granularity = 0
     * DB = 0 
     * Long mode = 1
     * Last 4 bits of the limit
    */
    .flags_and_limit = 0b00100000 | ((sizeof(tss_t) - 1) >> 16),

    /*
     * Present = 1
     * DPL = 00 
     * Segment = 0
     * Type = 0b1001 or TSS 64 (Available) 
    */
    .access_byte = 0b10001001, 

    .base0 = 0,
    .base1 = 0,
    .base2 = 0,
    .base3 = 0
  }
};

gdt64_descriptor_t gdt64_descriptor = {
  .size = sizeof(gdt64_t),
  .gdt64_addr = (uint64_t) &gdt64 
};


void remap_gdt(){
  gdt64.tss_segment.base0 = (uint16_t) ((uint64_t)&tss >> 0);
  gdt64.tss_segment.base1 = (uint8_t) ((uint64_t)&tss >> 16);
  gdt64.tss_segment.base2 = (uint8_t) ((uint64_t)&tss >> 24);
  gdt64.tss_segment.base3 = (uint32_t) ((uint64_t)&tss >> 32);
  
  tss.ist1 = ((uint64_t) alloc_kernel_stack()) + KERNEL_STACK_SIZE - 0x8;
  tss.rsp1 = ((uint64_t) alloc_kernel_stack()) + KERNEL_STACK_SIZE - 0x8;
   
  void* gdt_desc = (void*) &gdt64_descriptor; 
  uint16_t tss_offset = 0x28;
  asm("lgdt (%0)" : "=r"(gdt_desc)); 
  asm("ltr %0 " : : "a"(tss_offset));
}
