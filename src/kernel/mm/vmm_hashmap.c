/* vmm hashmap implementation 
 * The whole point of this is so I can walk the page tables 
 * I basically need a way to convert the physical addresses into virutal addresses
 * The only way to do that is to actually keep track of the page tables
*/
#include <vmm_hashmap.h>
#include <mem.h>
#include <kernel_heap_manager.h>
#include <libtitan.h>
#include <vmm.h>
#include <pmm.h>
#define MAX_FREED_TABLES 20

// This is a linked list that stores all of the freed nodes
page_table_t* tables = NULL;
vmm_hashmap_t vmm_hashmap = NULL;

extern vm_object kernel_page_tbls_obj;
extern uintptr_t* current_pml4t_ptr;

vmm_hashmap_t vmm_hashmap_init(){
  // This creates a new vmm_hashmap_t instance which is just a pointer into the hashmap buckets
  vmm_hashmap_t new_vmm = (page_table_t**) kmalloc(sizeof(page_table_t*) * HASH_MAP_BUCKETS);
  memset(new_vmm, 0, sizeof(page_table_t*) * HASH_MAP_BUCKETS);
  
  return new_vmm;
}


static size_t hash_function(void* paddr){
    return ((size_t) paddr) % HASH_MAP_BUCKETS;
}

static bool pg_tbl_insert(page_table_t* table){
  size_t bucket_index = hash_function(table->pt_paddr);
  page_table_t* current_node_ptr = vmm_hashmap[bucket_index];
  if(current_node_ptr == NULL){
    vmm_hashmap[bucket_index] = table; 
  }
  else{
    // Going til the end of the list
    while(current_node_ptr->next != NULL){
      current_node_ptr = current_node_ptr->next;
      if(current_node_ptr->pt_paddr == table->pt_paddr){
        current_node_ptr->pt_vaddr = table->pt_vaddr;
        return false;
      }
    }
    current_node_ptr->next = table;
  }
  return true;
}

page_table_t* get_freed_pg_tbl(){
  if(tables == NULL){
    return NULL;
  }

  // We just pop off the first node
  page_table_t* freed_tbl = tables;
  freed_tbl = freed_tbl->next;
  return freed_tbl;
}

void* alloc_pg_table(){
  page_table_t* table = get_freed_pg_tbl();
  if(!table){
    // We allocate a new page_tbl node
    table = kmalloc(sizeof(page_table_t));
    memset(table, 0, sizeof(page_table_t));
    if(!(table->pt_paddr = pmm_alloc_page())){
      return false; 
    }
    kernel_page_tbls_obj.length += 0x1000;

    table->pt_vaddr = (void*) (kernel_page_tbls_obj.base + kernel_page_tbls_obj.length);

    if(!mmap(current_pml4t_ptr, table->pt_paddr, table->pt_vaddr, VM_FLAG_NONE)){
      return false;
    }
  }

  memset(table->pt_vaddr, 0, PAGE_SIZE);

  if(!pg_tbl_insert(table)){
    return NULL;
  }
  return table->pt_paddr;
}

bool vmm_hashmap_put(vmm_hashmap_t vmm_hashmap, uint8_t level, void* pt_vaddr, void* pt_paddr){
  //You gotta run vmm_hashmap_init before running this function
  if(vmm_hashmap == NULL){
    return false;
  }

  if(tables != NULL){
     
  }
  
  page_table_t* new_node = kmalloc(sizeof(page_table_t));
  new_node->pt_vaddr = pt_vaddr;
  new_node->pt_paddr = pt_paddr;
  new_node->used = true;
  new_node->next = NULL;

  return pg_tbl_insert(new_node); 
}



void* get_ptbl_vaddr(vmm_hashmap_t vmm_hashmap, void* pt_paddr){
  if(vmm_hashmap == NULL){
    return false;
  }
  
  size_t bucket_index = hash_function(pt_paddr);
  page_table_t* current_node_ptr = vmm_hashmap[bucket_index];
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
  page_table_t* current_node_ptr = vmm_hashmap[bucket_index];
  if(current_node_ptr == NULL){
    goto failure;
  }
  else{
    // Going til the end of the list
    while(current_node_ptr->next != NULL){
      if(current_node_ptr->next->pt_paddr == pt_paddr){
        page_table_t* to_be_freed = current_node_ptr->next;
        current_node_ptr->next = current_node_ptr->next->next;
         
        to_be_freed->next = tables;
        tables = to_be_freed;
        return true;
      }
      current_node_ptr = current_node_ptr->next;
    }
  }

  failure:
    return false;
}
