#ifndef MB2_MMAP_H
#define MB2_MMAP_H
#include <stddef.h>
#include <stdint.h>
#include "multiboot2.h"

#define MAX_MMAP_ENTRIES 20

typedef struct multiboot_mmap_entry mmap_entry_t;

typedef struct mmap_region_list_t {
  size_t length;
  mmap_entry_t entries[MAX_MMAP_ENTRIES];
}mmap_region_list_t;

uintptr_t get_max_memory_addr();
bool mb2_mmap_init(struct multiboot_info* mb_info);
bool check_region_reserved(void* region_start, size_t length);
mmap_region_list_t* get_reserved_entries();
#endif // !MB2_MMAP_H
