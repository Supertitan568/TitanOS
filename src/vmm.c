#include "mem.h"
#include "pmm.h"
#include "vmm.h"
#include "vmm_hashmap.h"
#include "kernel_heap_manager.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// Bits 0 - 11 are page flags 


#define KERNEL_SIZE 0x6000

// This makes it so it rounds up to the nearest page boundry
#define PAGE_ALIGN(n) (((n) + 0xfff) / 0x1000) * 0x1000

// Using this to keep track of the page table level this thing has
struct page_table {
  uint8_t level;
  void* ptable;
  struct page_table* next;
};

// Literally just the kernel size from the makefile. We should get this from the linker
typedef struct vm_object{
  uintptr_t base;
  size_t length;
  size_t flags;
  struct vm_object* next;
}vm_object;

// My god I am using hella linked lists. This is more than I will probably ever use again.
// This is the inital linked list for the kernel
vm_object kernel_page_tbls_obj = {
  0xffffff8000200000,
  0x6000,
  VM_FLAG_READ_WRITE,
  NULL
};

vm_object kernel_stack_obj = {
  0xffffff80001fa000,
  0x6000,
  VM_FLAG_READ_WRITE,
  &kernel_page_tbls_obj
};


vm_object kernel_heap_obj = {
  0xffffff8000006000,
  0x20000,
  VM_FLAG_READ_WRITE,
  &kernel_stack_obj
};

vm_object kernel_text_obj = {
  0xffffff8000000000,
  KERNEL_SIZE,
  VM_FLAG_READ_WRITE,
  &kernel_heap_obj
};

// We need this dummy_proc object at 0x0000 to start our vm_object list
// TODO: Change this to be length 0 once we get this working
vm_object head_obj  = {
  0x00000000,
  0x26000,
  VM_FLAG_READ_WRITE,
  &kernel_text_obj
};


static inline uint16_t get_index(uintptr_t addr, uint8_t table_level){
  // This gets the index of the block in the page table we are looking at
  return (( addr >> (9 * table_level + 12)) & 0x1ff);
}


static inline uintptr_t get_block_vaddr(uintptr_t table_virt_addr, uint16_t table_index, uint8_t table_level){
  // This gets the begining address of what the block controls
  return table_virt_addr + (table_index << (9 * table_level  + 12)); 
}


void umap(uint64_t* table_ptr, uintptr_t table_virt_addr, uint8_t table_level, uintptr_t region_start_addr, uintptr_t region_end_addr){
  // start and end indexes of the region we are looking at
  uint16_t start_index = get_index(region_start_addr, table_level);
  uint16_t end_index = get_index(region_start_addr, table_level);

  for(uint16_t i = start_index; i < end_index; i++){
    // basically if the table is higher than a pt
    uintptr_t pt_phys_addr = (uintptr_t) (*(table_ptr + i) & 0xfffffffffffff000);

    if(table_level){
      // Addresses of the current block 
      uintptr_t block_start_addr = get_block_vaddr(table_virt_addr, i, table_level);
      uintptr_t block_end_addr = get_block_vaddr(table_virt_addr, i + 1, table_level);

      // This is the bounds of the region for the sub block we are looking at
      uintptr_t start_addr = region_start_addr;
      uintptr_t end_addr = region_end_addr;
      
      // We need to see if the bounds we are looking at in the main function are in the sub block
      if(region_start_addr < block_start_addr){
        start_addr = block_start_addr;
      }
      if(region_start_addr < block_start_addr){
        start_addr = block_start_addr;
      }
      // Doing this same thing for the sub block
      umap(get_ptbl_vaddr((void*) pt_phys_addr), get_block_vaddr(table_virt_addr, i, table_level), table_level - 1, start_addr, end_addr);

      // If the entire block is in the region we need to free it 
      if(region_start_addr <= block_start_addr && region_end_addr >= block_end_addr){
        pmm_free_page((void*) (*(table_ptr + i) & 0xfffffffffffff000));
        //vmm_hashmap_remove((void*) (*(table_ptr + i) & 0xfffffffffffff000));
        *(table_ptr + i) = 0;
      }
    }
    // This is for when we are at the bottom level pt. We are finally at the lowest region we can find
    else {
      // Clear the entry
      pmm_free_page((void*) (*(table_ptr + i) & 0xfffffffffffff000));
      //vmm_hashmap_remove((void*) (*(table_ptr + i) & 0xfffffffffffff000));
      *(table_ptr + i) = 0;
    }
  }
}

bool mmap(uint64_t* root_table, void* paddr, void* vaddr, size_t flags){
  uintptr_t pt_offset;
  uint64_t pt_value;
  
  uint64_t* pt_vaddr = root_table;
  void* pt_paddr;

  for(int pt_level = 3; pt_level > 0; pt_level--){
    pt_offset = ((((uintptr_t) vaddr) >> (9 * pt_level + 12)) & 0x1ff);
    pt_value = pt_vaddr[pt_offset];
    // If we don't have the page table we need to create it and make the entry
    if(pt_value & 1){
      // In this case there is a valid pt entry
      pt_paddr = (void*) (pt_value & 0xfffffffffffff000);
      if(! (pt_vaddr = get_ptbl_vaddr(pt_paddr))){
        return false;
      }
    }
    else{
      // We need to create the new page table in this case
      if(!(pt_paddr = pmm_alloc_page())){
        return false; 
      }

      kernel_page_tbls_obj.length += 0x1000;
      vmm_hashmap_put(1, (void*) (kernel_page_tbls_obj.base + kernel_page_tbls_obj.length), pt_paddr);
      // Is this recursion a code smell? Maybe. It works tho
      if(!mmap(root_table, pt_paddr, (void*) (kernel_page_tbls_obj.base + kernel_page_tbls_obj.length) , 0)){
        return false;
      }
      pt_vaddr[pt_offset] = ((uint64_t) pt_paddr) | 0x3;
      pt_vaddr = (void*) (kernel_page_tbls_obj.base + kernel_page_tbls_obj.length); 
      memset(pt_vaddr, 0, 0x1000);
      
    }

  }

  // After this we have the bottom pt. We just have to write our entry in
  pt_offset = (((uintptr_t) vaddr) >> 12) & 0x1ff;
  pt_vaddr[pt_offset] = ((uint64_t) paddr) | VM_FLAG_MMIO | 0x3;
  return true;
}

bool vmm_lengthen(uintptr_t start_region, int inc){
  // We can only work with pages so the lengths in the vmm obj needs to be page aligned
  inc = PAGE_ALIGN(inc);

  vm_object* curr_vm_obj = &head_obj;
  
  // We look through the list for the requested region
  while(curr_vm_obj != NULL){
      if(curr_vm_obj->base == start_region){
        break;  
      } 
      curr_vm_obj = curr_vm_obj->next;
  }
  
  // If we dont find it it will be null
  if(curr_vm_obj == NULL){
    return false;
  }
  
  // If there is something in front of our vm_obj we have to check if there is room for the new length
  if(curr_vm_obj->next != NULL){
    if(!((curr_vm_obj->base + curr_vm_obj->length + inc) <= curr_vm_obj->next->base)){
      return false;
    }
  }
  
  // There should be enough length if we made it here
  uintptr_t pt_vaddr = curr_vm_obj->base + curr_vm_obj->length;
  curr_vm_obj->length += inc;
  void* pt_paddr;

  for(; pt_vaddr < curr_vm_obj->base + curr_vm_obj->length; pt_vaddr += 0x1000){
    if(curr_vm_obj->flags & VM_FLAG_MMIO){
      // TODO: Add code for handling lengthing mmio stuff 
      // pt_paddr = args;
      // pmm_alloc_specific_page(pt_paddr);
      return false;
    }
    else{
      pt_paddr = pmm_alloc_page(); 
    }

    if(!mmap((void*) PML4T_ADDR, pt_paddr,(void*) pt_vaddr, curr_vm_obj->flags)){
      return NULL;
    }
  }  
  
  return true;
}

void vmm_free(void* start_region){
  // I'm going to assume for right now that this exists in here and it has been allocated with kmalloc
  // TODO: get rid of those assumptions
  vm_object* curr_vmm_obj = &head_obj;
  while(((void*) curr_vmm_obj->next->base) != start_region){
    curr_vmm_obj = curr_vmm_obj->next; 
  }
  
  vm_object* tmp_ptr = curr_vmm_obj->next;
  curr_vmm_obj->next = curr_vmm_obj->next->next;
  kfree(tmp_ptr);
}

void* vmm_alloc(uintptr_t start_region, size_t length, size_t flags, void* args){
  length = PAGE_ALIGN(length);

  // Checks to see if the region requested already exists and if it does then just lengthens it else returns an error
  vm_object* current_vm_object = &head_obj;
  uintptr_t found;
  if((void*) start_region == NULL){
    // Going through the list until we find a space with enough space to put a region with the requested size
    // We are just using first fit here
    current_vm_object = &head_obj;
    uintptr_t base;
    while(current_vm_object->next != NULL){
      base = current_vm_object->base + current_vm_object->length;
      if(base + length < current_vm_object->next->base){
        found = base;
        break;
      }
      current_vm_object = current_vm_object->next;
    } 
  }
  else{
    // We first want to check to make sure the requested region is not already allocated
    while(current_vm_object->next != NULL){
      // Let's see if the requested region is in between this region and the next
      if(current_vm_object->base <=  start_region && start_region < current_vm_object->next->base){
        // Let's check to make sure that we have enough space too
        if(!(((current_vm_object->base + current_vm_object->length) <=  start_region) && (start_region + length) <= (uintptr_t) current_vm_object->next)){
          return NULL;
        }
        
        found = start_region;
        break;
      } 
      current_vm_object = current_vm_object->next;
    }
  }
  
  
  // Creating the new vmm object and inserting it into the list 
  vm_object* new_region = (vm_object*) kmalloc(sizeof(vm_object));
  new_region->base = found;
  new_region->length = length;
  new_region->flags = flags;

  new_region->next = current_vm_object->next;
  current_vm_object->next = new_region;

  // Backing this new region with new page table
  void* pt_paddr;
  void* pt_vaddr = (void*) found;
  // Right now I am assuming that this is not allocated
  // TODO: Get rid of that assumption 

  for(int i = 0; i < (length + 0xfff) / 0x1000; i++){
    if(flags & VM_FLAG_MMIO){
      pt_paddr = args;
      pmm_alloc_specific_page(pt_paddr);
    }
    else{
      pt_paddr = pmm_alloc_page(); 
    }

    if(!mmap((void*) PML4T_ADDR, pt_paddr,  pt_vaddr, flags)){}}
      return NULL;
    }
    pt_vaddr += 0x1000;
  }

  return (void*) found;
}



void vmm_init(){
  kheap_init();
  pmm_init(0xfed00);
  *((uint64_t*)0xffffff8000200000) = 0;
  vmm_hashmap_init();
  vmm_hashmap_put(1, (void*) 0xffffff8000200000, (void*) 0xe000);

  vmm_hashmap_put(1, (void*) 0xffffff8000201000, (void*) 0xf000);
  vmm_hashmap_put(1, (void*) 0xffffff8000202000, (void*) 0x10000);
  vmm_hashmap_put(1, (void*) 0xffffff8000203000, (void*) 0x11000);
  vmm_hashmap_put(1, (void*) 0xffffff8000204000, (void*) 0x12000);
}
