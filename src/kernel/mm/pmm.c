#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "pmm.h"
#include "libtitan.h"
#include "mem.h"
#include "kernel_heap_manager.h"
#include "vmm.h"
#include "mb2_mmap.h"


// This is literally just a bit map with each entry being 64 or 0x20 pages
struct {
  size_t num_pages; // Number of pages this can address, not the number of pages in the bitmap lol
  uint64_t* bitmap;
} map;

typedef struct bitmap_coords{
  uint32_t y;
  uint8_t x;
}bitmap_coords;

#define TMP_PMM_BYTE_SIZE KERNEL_HEAP_START - (KERNEL_PMM_START) 

void pmm_init(){
    //map.num_pages = get_max_memory_addr() / PAGE_SIZE;
    //size_t map_byte_size = map.num_pages / 8;
    size_t map_byte_size = TMP_PMM_BYTE_SIZE;
    map.num_pages = map_byte_size * 8;
    
    // We get the end of the kernel + mb2 struct + kernel heap and page align it down
    //uintptr_t pmm_first_page = (get_kernel_phys_end() + PAGE_SIZE) & 0xfffffffffffff000;
    map.bitmap = (uint64_t*) (KERNEL_PMM_START);
    
    // We first setup the first page of the pmm
    memset((void*) map.bitmap, 0, PAGE_SIZE);
    
    // We then go through and mark everything we need reserved
    // We start with the reserved memory entries from mb2_info
    mmap_region_list_t* reserved_entries = get_reserved_entries();   
    mmap_entry_t* e = reserved_entries->entries;
    for(;e != (reserved_entries->entries + reserved_entries->length) || (e->addr + e->len) >= (PAGE_SIZE * 8 * PAGE_SIZE); e++){
      for(uintptr_t p = e->addr; p > (e->addr + e->len); p += PAGE_SIZE){
        pmm_alloc_specific_page((void*) p);
      } 

    }
    
    // We then mark the kernel, the kernel heap, and the first page of this pmm
    for(uintptr_t p = (uintptr_t) KERNEL_PHYS; p < ((uintptr_t) kernel_byte_size) + ((uintptr_t) KERNEL_PHYS) + PAGE_SIZE * 2; p += PAGE_SIZE){
      pmm_alloc_specific_page((void*)p);
    }
    
    // if(check_region_reserved((void*) map.bitmap, map_byte_size)){
    //   PANIC("pmm", "physical memory after kernel text is reserved");
    // }
    
    if(map_byte_size <= PAGE_SIZE){
      return;
    }

    if(!vmm_lengthen((uintptr_t) map.bitmap, map_byte_size - PAGE_SIZE)){
      ERROR("pmm", "bitmap lengthening failed");
      map.num_pages = 8 * 0x1000;
    }
}


static void* bitmap_to_paddr(uint32_t x, uint32_t y){
  return (void*) ((x * 0x1000) + (y * 0x20000));
}


static bitmap_coords paddr_to_bitmap(void* paddr){
  uint64_t phys_addr = ((uint64_t) paddr) / 0x1000;

  uint32_t y = phys_addr / 64;
  uint8_t x = phys_addr % 64;
  
  bitmap_coords coords = {y, x};
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
  bitmap_coords coords = paddr_to_bitmap(paddr);
  uint64_t masked = (map.bitmap[coords.y]) & (1 << coords.x);

  return masked > 0;
}


bool pmm_alloc_specific_page(void* paddr){
  // We really need this for MMIO
  bitmap_coords coords = paddr_to_bitmap(paddr);
   
  if((map.bitmap[coords.y]) & (1 << coords.x)){
    return true;
  }
  else{
    map.bitmap[coords.y] |= (1 << coords.x);
    return false;
  }

}


void pmm_free_page(void* paddr){
  bitmap_coords coords = paddr_to_bitmap(paddr);
  map.bitmap[coords.y] &= 0xffffffffffffffff ^ (1 << coords.x);  
}
