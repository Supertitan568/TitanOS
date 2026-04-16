#ifndef VMM_H
#define VMM_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "isr.h"

extern char kernel_byte_size[];
extern char initial_pml4t[];
#define INITIAL_PML4T_ADDR (&initial_pml4t) 
#define VM_FLAG_NONE 0
#define VM_FLAG_READ_WRITE 1
#define VM_FLAG_USER (1 << 1)
#define VM_FLAG_MMIO (1 << 4)

#define KERNEL_PAGE_TBL_START 0xffffff8000300000
#define KERNEL_STACK_START 0xffffff80002fe000
#define KERNEL_HEAP_START 0xffffff8000200000
#define KERNEL_PMM_START VGA_TEXT_BUFFER_BASE + 0x28000
#define VGA_TEXT_BUFFER_BASE 0xffffff80000c0000
#define KERNEL_TEXT_START 0xffffff8000000000

#define KERNEL_STACK_SIZE 0x2000
#define KERNEL_SIZE kernel_byte_size

#define USER_STACK_START 0xc0000000

typedef struct vm_obj{
  uintptr_t base;
  size_t length;
  size_t flags;
  struct vm_obj* next;
}vm_object;

typedef struct{
  vm_object head;
  uint32_t current_vmm_epoch;
  uint32_t previous_vmm_epoch;
  void* pml4t;
} vmm_t; 

typedef struct{
  uint8_t type;
  size_t flags;
#define CODE_SECTION 0
#define STACK_SECTION 1
  void* start;
  size_t length;
} section_t;

typedef struct{
  uint8_t type;
  size_t flags;
#define CODE_SECTION 0
#define STACK_SECTION 1
  void* start; 
  size_t length;
  void* source_start;
} code_section_t;

void* alloc_kernel_stack();
void* alloc_user_stack(void* user_stack, vmm_t* vmm);
void create_sections(section_t** section_list, size_t length);
uintptr_t get_mmio_ptr(size_t length);
void* alloc_mmio(void* phys_region_start, size_t length, size_t extra_flags);
void* vmm_alloc(uintptr_t start_region, size_t length, size_t flags, void* args);
void vmm_set_region_length(void* region, size_t length);
vmm_t vmm_init();
bool mmap(uint64_t* root_table, void* paddr, void* vaddr, size_t flags);
void vmm_free(void* start_region);
bool vmm_lengthen(uintptr_t start_region, int inc);
void switch_vm_space(vmm_t* vmm);
vmm_t create_vmm(vmm_t current_vmm);
#endif // !VMM_H
