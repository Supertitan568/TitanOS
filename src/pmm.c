#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "pmm.h"
#include "mem.h"
#include "kernel_heap_manager.h"

// This is literally just a bit map with each entry being 64 or 0x20 pages
struct mem_map {
  size_t num_pages;
  uint64_t* bitmap;
} map;

struct bitmap_coords{
  uint32_t y;
  uint8_t x;
};

void pmm_init(size_t num_pages){
    map.num_pages = num_pages;
    // We have to put in the amount of bytes not bits (8 not 64) :P
    map.bitmap = kmalloc(num_pages / 8);
    mem_set((void*) map.bitmap, 0, num_pages / 8);    
    // Allocating all of the pages that are going to be used at boot here
    for(int i = 0; i < 64; i++)
      pmm_alloc_specific_page((void*) (i * 0x1000));

}


static void* bitmap_to_paddr(uint32_t x, uint32_t y){
  return (void*) ((x * 0x1000) + (y * 0x20000));
}


static struct bitmap_coords paddr_to_bitmap(void* paddr){
  uint64_t phys_addr = ((uint64_t) paddr) / 0x1000;

  uint32_t y = phys_addr / 64;
  uint8_t x = phys_addr % 64;
  
  struct bitmap_coords coords = {y, x};
  return coords;
}


void* pmm_alloc_page(){
  for(uint32_t i = 0; i < (map.num_pages / 64); i++){
    if(map.bitmap[i] < 0xffffffffffffffff){
      uint32_t j = 0;
      while(j < 64){
        if(!(map.bitmap[i] & (1 << j))){
          map.bitmap[i] |= (1 << j); 
          return bitmap_to_paddr(j, i);
        }
        j++;
      }
    }
  }

  return 0;
}


bool pmm_check_page_taken(void* paddr){
  struct bitmap_coords coords = paddr_to_bitmap(paddr);
  uint64_t masked = (map.bitmap[coords.y]) & (1 << coords.x);

  return masked > 0;
}


bool pmm_alloc_specific_page(void* paddr){
  // We really need this for MMIO
  struct bitmap_coords coords = paddr_to_bitmap(paddr);
   
  if((map.bitmap[coords.y]) & (1 << coords.x)){
    return true;
  }
  else{
    map.bitmap[coords.y] |= (1 << coords.x);
    return false;
  }

}


void pmm_free_page(void* paddr){
  struct bitmap_coords coords = paddr_to_bitmap(paddr);
  map.bitmap[coords.y] &= 0xffffffffffffffff ^ (1 << coords.x);  
}
