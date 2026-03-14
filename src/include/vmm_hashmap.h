#ifndef VMM_HASHMAP_H
#define VMM_HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HASH_MAP_BUCKETS 20

struct page_table {
  uint8_t level;
  void* pt_vaddr;
  void* pt_paddr;
  struct page_table* next;
};

typedef struct page_table** vmm_hashmap_t;

vmm_hashmap_t vmm_hashmap_init();
bool vmm_hashmap_put(vmm_hashmap_t vmm_hashmap, uint8_t level, void* pt_vaddr, void* pt_paddr);
void* get_ptbl_vaddr(vmm_hashmap_t vmm_hashmap, void* pt_paddr);
bool vmm_hashmap_remove(vmm_hashmap_t vmm_hashmap, void* pt_paddr);


#endif // !VMM_HASHMAP_H
