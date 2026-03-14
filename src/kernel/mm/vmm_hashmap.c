/* vmm hashmap implementation 
 * The whole point of this is so I can walk the page tables 
 * I basically need a way to convert the physical addresses into virutal addresses
 * The only way to do that is to actually keep track of the page tables
*/
#include "vmm_hashmap.h"
#include "mem.h"
#include "vga.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel_heap_manager.h"

vmm_hashmap_t vmm_hashmap_init(){
  // This creates a new vmm_hashmap_t instance which is just a pointer into the hashmap buckets
  vmm_hashmap_t new_vmm = (struct page_table**) kmalloc(sizeof(struct page_table*) * HASH_MAP_BUCKETS);
  memset(new_vmm, 0, sizeof(struct page_table*) * HASH_MAP_BUCKETS);
  
  return new_vmm;
}


static size_t hash_function(void* paddr){
    return ((size_t) paddr) % HASH_MAP_BUCKETS;
}


bool vmm_hashmap_put(vmm_hashmap_t vmm_hashmap, uint8_t level, void* pt_vaddr, void* pt_paddr){
  //You gotta run vmm_hashmap_init before running this function
  if(vmm_hashmap == NULL){
    return false;
  }
  
  struct page_table* new_node = kmalloc(sizeof(struct page_table));
  new_node->pt_vaddr = pt_vaddr;
  new_node->pt_paddr = pt_paddr;
  new_node->next = NULL;

  size_t bucket_index = hash_function(pt_paddr);
  struct page_table* current_node_ptr = vmm_hashmap[bucket_index];
  if(current_node_ptr == NULL){
    vmm_hashmap[bucket_index] = new_node; 
  }
  else{
    // Going til the end of the list
    while(current_node_ptr->next != NULL){
      current_node_ptr = current_node_ptr->next;
      if(current_node_ptr->pt_paddr == pt_paddr){
        current_node_ptr->pt_vaddr = pt_vaddr;
        kfree(new_node);
        return true;
      }
    }
    current_node_ptr->next = new_node;
  }
  return true;
}

void* get_ptbl_vaddr(vmm_hashmap_t vmm_hashmap, void* pt_paddr){
  if(vmm_hashmap == NULL){
    return false;
  }
  
  size_t bucket_index = hash_function(pt_paddr);
  struct page_table* current_node_ptr = vmm_hashmap[bucket_index];
  if(current_node_ptr == NULL){
    return NULL;
  }
  else{
    while(current_node_ptr != NULL){
      if(current_node_ptr->pt_paddr == pt_paddr){
        return current_node_ptr->pt_vaddr; 
      }
      current_node_ptr = current_node_ptr->next;
    }
  }
  return NULL;
}

bool vmm_hashmap_remove(vmm_hashmap_t vmm_hashmap, void* pt_paddr){
  if(vmm_hashmap == NULL){
    return false;
  }

  size_t bucket_index = hash_function(pt_paddr);
  struct page_table* current_node_ptr = vmm_hashmap[bucket_index];
  if(current_node_ptr == NULL){
    goto failure;
  }
  else{
    // Going til the end of the list
    while(current_node_ptr->next != NULL){
      if(current_node_ptr->next->pt_paddr == pt_paddr){
        void* to_be_freed = current_node_ptr->next;
        current_node_ptr->next = current_node_ptr->next->next;
        kfree(current_node_ptr->next);
      }
      current_node_ptr = current_node_ptr->next;
    }
  }

  failure:
    return false;
}
