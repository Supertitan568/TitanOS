#include <stdint.h>
#include <stdbool.h>

#include "pmm.h"
#include "vmm.h"
#include "kernel_heap_manager.h"

#define HEAP_START 0xffffff8000006000 
#define INITAL_HEAP_SPACE_END 0xffffff8000026000;

#define PAGE_ROUND(n) ((n) + 0xfff) / 0x1000

typedef struct __attribute__((packed)) {
  bool used;
  uint32_t size;
  uint8_t region[];
} memory_region_t;

memory_region_t* heap_start = (memory_region_t*) HEAP_START;
uint8_t* heap_end = (uint8_t*) INITAL_HEAP_SPACE_END;


static inline memory_region_t* go_to_next_memory_region(memory_region_t* curr_mem_region){
  return (memory_region_t*) (((uint8_t*) curr_mem_region) + curr_mem_region->size + 5);
}

static inline void* add_void_ptr(void* ptr, uintptr_t num){
  return (void*) (((uintptr_t) ptr) + num); 
}


static bool get_more_pages(int num_pages){
  int region_length = ((uintptr_t) heap_end) - ((uintptr_t) heap_start) + (0x1000 * num_pages);
  if(vmm_lengthen((uintptr_t) heap_start, 0x1000)){
    heap_end += (0x1000 * num_pages);
    return true;
  }
  return false;
}


static inline void write_new_mem_reg(memory_region_t* new_region){
  new_region->used = false;
  new_region->size = ((uintptr_t) heap_end) - ((uintptr_t) new_region);
}


void* kmalloc(size_t size){
  // TODO: Refactor this function to make it more simple
  memory_region_t* curr_mem_region = heap_start;
  
  while(true){
    if(curr_mem_region >= heap_end){
      // Theoretically if we get here it should still be aligned with the previous mem region
      get_more_pages(1);
      write_new_mem_reg(curr_mem_region);
    }
    // Case 1: We ran into a used memory region
    if(curr_mem_region->used){
      curr_mem_region = go_to_next_memory_region(curr_mem_region);
      continue;
    }
    // Case 2: We find a good sized block
    else if (!curr_mem_region->used && curr_mem_region->size >= size) {
      curr_mem_region->used = true;
      // If we have enough space to write the mem region header, we are going to write it
      if(curr_mem_region->size - sizeof(memory_region_t) > size){
        uint32_t old_size = curr_mem_region->size;
        curr_mem_region->size = size;

        memory_region_t* next_mem_region = go_to_next_memory_region(curr_mem_region);

        next_mem_region->used = false;
        next_mem_region->size = old_size - size - sizeof(memory_region_t);
      }
       
      return add_void_ptr(curr_mem_region, sizeof(memory_region_t)); 
    }
    // Case 3: We find a free block, but it's not the right size
    else{
      uint32_t mem_reg_size = curr_mem_region->size;
      memory_region_t* next_mem_region = go_to_next_memory_region(curr_mem_region);
      if(next_mem_region >= heap_end){
          // Theoretically if we get here it should still be aligned with the previous mem region
          get_more_pages(PAGE_ROUND(size - mem_reg_size));
          memory_region_t new_region = {
            false,
            PAGE_ROUND(size - mem_reg_size) * 0x1000,
          };
          write_new_mem_reg(&new_region);
      } 
      // We go until we get enough contiguous free blocks for our space
      while(!next_mem_region->used){
        mem_reg_size += next_mem_region->size + 5;
        if(mem_reg_size >= size){
          // We found enough consecutive blocks
          curr_mem_region->used = true;
          if(mem_reg_size - 5 > size){
            curr_mem_region->size = size;
            next_mem_region = go_to_next_memory_region(curr_mem_region);
            next_mem_region->used = false;
            next_mem_region->size = mem_reg_size - size - 5;
          }
          return add_void_ptr(curr_mem_region, sizeof(memory_region_t));
        }
        next_mem_region = go_to_next_memory_region(next_mem_region);
        
        if(next_mem_region >= heap_end){
          // Theoretically if we get here it should still be aligned with the previous mem region
          get_more_pages(PAGE_ROUND(size - mem_reg_size));
          memory_region_t new_region = {
            false,
            PAGE_ROUND(size - mem_reg_size) * 0x1000,
          };
          write_new_mem_reg(&new_region);
        } 
      }
      curr_mem_region = go_to_next_memory_region(next_mem_region);
    }
  }
}


void kfree(void* ptr){
  ((memory_region_t*)(((uint8_t*) ptr) - sizeof(memory_region_t)))->used = false;
  // TODO: Add a way to get free some pages
}


void kheap_init(){
  heap_start->used = false;
  heap_start->size = (uint32_t) ((uintptr_t) heap_end) - ((uintptr_t) heap_start) - 5;
}
