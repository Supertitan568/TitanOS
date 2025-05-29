#ifndef VMM_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PML4T_ADDR 0xffffff8000200000
#define VM_FLAG_NONE 0
#define VM_FLAG_READ_WRITE 1
#define VM_FLAG_USER (1 << 1)
#define VM_FLAG_MMIO (1 << 2)

void* vmm_alloc(uintptr_t start_region, size_t length, size_t flags, void* args);
void vmm_set_region_length(void* region, size_t length);
void vmm_init();
bool mmap(uint64_t* root_table, void* paddr, void* vaddr, size_t flags);
void vmm_free(void* start_region);
bool vmm_lengthen(uintptr_t start_region, int inc);

#endif // !VMM_H
