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
#define HASH_MAP_BUCKETS 20

struct page_table {
  uint8_t level;
  void* pt_vaddr;
  void* pt_paddr;
  struct page_table* next;
};

struct page_table** hashmap_buckets = NULL;
 
void vmm_hashmap_init(){
  hashmap_buckets = (struct page_table**) kmalloc(sizeof(struct page_table*) * HASH_MAP_BUCKETS);
  // printstr("Page Table Struct length: ");
  // printlong(sizeof(struct page_table));
  // printc('\n');
  mem_set(hashmap_buckets, 0, sizeof(struct page_table*) * HASH_MAP_BUCKETS);
}


static size_t hash_function(void* paddr){
    return ((size_t) paddr) % HASH_MAP_BUCKETS;
}


bool vmm_hashmap_put(uint8_t level, void* pt_vaddr, void* pt_paddr){
  //You gotta run vmm_hashmap_init before running this function
  if(hashmap_buckets == NULL){
    return false;
  }
  
  struct page_table* new_node = kmalloc(sizeof(struct page_table));
  new_node->pt_vaddr = pt_vaddr;
  new_node->pt_paddr = pt_paddr;
  new_node->next = NULL;

  size_t bucket_index = hash_function(pt_paddr);
  struct page_table* current_node_ptr = hashmap_buckets[bucket_index];
  if(current_node_ptr == NULL){
    hashmap_buckets[bucket_index] = new_node; 
  }
  else{
    // Going til the end of the list
    while(current_node_ptr->next != NULL){
      current_node_ptr = current_node_ptr->next;
    }
    current_node_ptr->next = new_node;
  }
  return true;
}

void* get_ptbl_vaddr(void* pt_paddr){
  if(hashmap_buckets == NULL){
    return false;
  }
  
  size_t bucket_index = hash_function(pt_paddr);
  struct page_table* current_node_ptr = hashmap_buckets[bucket_index];
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
