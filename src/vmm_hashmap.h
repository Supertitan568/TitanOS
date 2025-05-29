#ifndef VMM_HASHMAP_H
#define VMM_HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void vmm_hashmap_init();
bool vmm_hashmap_put(uint8_t level, void* pt_vaddr, void* pt_paddr);
void* get_ptbl_vaddr(void* pt_paddr);

#endif // !VMM_HASHMAP_H
