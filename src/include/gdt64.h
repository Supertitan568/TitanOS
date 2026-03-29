#ifndef GDT64
#define GDT64

#include <stdint.h>
#include "vmm.h"
#include <stdint.h>

#define CODE_OFFSET 0x8
#define DATA_OFFSET 0x10
#define USER_CODE_OFFSET 0x1b
#define USER_DATA_OFFSET 0x23
#define TSS_OFFSET 0x28
typedef struct __attribute__((packed)) gdt64_segment_t{
  uint16_t limit;          // Max addressable unit
  uint16_t base0;          // First 2 bytes of the base address  
  uint8_t base1;           // Next byte of the base address
  uint8_t access_byte;     // Defines the type, priveledge level, etc...
  uint8_t flags_and_limit; // See osdev wiki for this one
  uint8_t base2;           // Another byte of the base address
  uint32_t base3;          // Another 4 bytes of the base address
  uint32_t reserved;       // I'm not sure what this is for
} gdt64_segment_t;

typedef struct __attribute__((packed)) gdt_segment_t{
  uint16_t limit;          // Max addressable unit
  uint16_t base0;          // First 2 bytes of the base address  
  uint8_t base1;           // Next byte of the base address
  uint8_t access_byte;     // Defines the type, priveledge level, etc...
  uint8_t flags_and_limit; // See osdev wiki for this one
  uint8_t base2;           // Last byte of 32 bit address 
} gdt_segment_t;

typedef struct __attribute__((packed)) gdt64_t{
  gdt_segment_t null_segment;
  gdt_segment_t code_segment;
  gdt_segment_t data_segment;
  gdt_segment_t user_code_segment;
  gdt_segment_t user_data_segment;
  gdt64_segment_t tss_segment;
} gdt64_t;

typedef struct __attribute__((packed)) gdt64_descriptor_t{
  uint16_t size;     // Size of the gdt;
  uint64_t gdt64_addr; // Address of the gdt;
} gdt64_descriptor_t;

typedef struct __attribute__ ((packed, aligned(16))) tss_t {
  uint32_t res0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t res1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t res2;
  uint16_t res3;
  uint16_t iopb;
} tss_t;

void remap_gdt();

#endif // !GDT64        
