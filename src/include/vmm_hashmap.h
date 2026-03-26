#ifndef VMM_HASHMAP_H
#define VMM_HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HASH_MAP_BUCKETS 20

typedef struct page_table_t {
  uint8_t level;
  void* pt_vaddr;
  void* pt_paddr;
  bool used;
  struct page_table_t* next;
}page_table_t;

typedef page_table_t** vmm_hashmap_t;

vmm_hashmap_t vmm_hashmap_init();
bool vmm_hashmap_put(vmm_hashmap_t vmm_hashmap, uint8_t level, void* pt_vaddr, void* pt_paddr);
void* get_ptbl_vaddr(vmm_hashmap_t vmm_hashmap, void* pt_paddr);
bool vmm_hashmap_remove(vmm_hashmap_t vmm_hashmap, void* pt_paddr);

void* alloc_pg_table();

#endif // !VMM_HASHMAP_H
