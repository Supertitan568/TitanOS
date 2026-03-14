#include <stddef.h>
#include <stdbool.h>
#include "multiboot2.h"
#include <libtitan.h>
#include <mb2_mmap.h>

//region 1 overlaps region 2
#define WITHIN(r1, r2, l1, l2) ((r1 + l1) > (r2) && (r1) < (r2)) || ((r2 + l2) > (r1) && (r2) < (r1))



mmap_region_list_t available_entries = {
  .length = 0,
  .entries = {}
};
mmap_region_list_t reserved_entries = {
  .length = 0,
  .entries = {}
};

mmap_region_list_t* get_reserved_entries(){
  return &reserved_entries;
}

bool check_region_reserved(void* region_start, size_t length){
  mmap_entry_t* e = reserved_entries.entries;
  for(int i = 0 ; i < reserved_entries.length; i++){
    if(WITHIN((uintptr_t)region_start,(uintptr_t) e->addr, length, e->len)){
      return true;
    }    
  }
  return false;
}

uintptr_t get_max_memory_addr(){
  uintptr_t max_addr = 0;
  for(mmap_entry_t* e = available_entries.entries; e != available_entries.entries + available_entries.length; e++){
    if(e->addr + e->len > max_addr){
      max_addr = e->addr + e->len;
    }
  }

  for(mmap_entry_t* e = reserved_entries.entries; e != reserved_entries.entries + reserved_entries.length; e++){
    if(e->addr + e->len > max_addr){
      max_addr = e->addr + e->len;
    }
  }

  return max_addr;
}

void dump_regions(){
  printf("Available Regions:\n");
  for(mmap_entry_t* e = available_entries.entries; e < available_entries.entries + available_entries.length; e++){
    printf("Start Address: %lu; Length : %lu\n", (uint64_t) e->addr, (uint64_t) e->len);
  }

  printf("Reserved Regions:\n");
  for(mmap_entry_t* e = reserved_entries.entries; e < reserved_entries.entries + reserved_entries.length; e++){
    printf("Start Address: %lu; Length : %lu\n", (uint64_t) e->addr, (uint64_t) e->len);
  }
}

bool mb2_mmap_init(struct multiboot_info* mb_info){
  // Finds memory map tag
  struct multiboot_tag_mmap* tag = (struct multiboot_tag_mmap*) mb_info->tags;
  for(; tag->type != MULTIBOOT_HEADER_TAG_END && tag->type != MULTIBOOT_TAG_TYPE_MMAP; tag = (struct multiboot_tag_mmap*) ALIGN_UP(((uintptr_t) tag) + tag->size, MULTIBOOT_INFO_ALIGN));

  if(tag->type != MULTIBOOT_TAG_TYPE_MMAP){
    PANIC("mb2_mmap", "Tag of Unknown type");
  }
  
  size_t memory_size = 0;
  // Inserts each mmap entry in either reserved or available
  for(struct multiboot_mmap_entry* e = tag->entries; ((uintptr_t) e) < ((uintptr_t) tag + tag->size); e = (struct multiboot_mmap_entry*)(((uintptr_t) e) + tag->entry_size)){
    mmap_region_list_t* list;
    if(e->type == MULTIBOOT_MEMORY_AVAILABLE){
      list = &available_entries;
      memory_size += e->len;
    }
    else{
      list = &reserved_entries;
    }
    //ASSERT(list->length < MAX_MMAP_ENTRIES);
    list->entries[list->length] = *e;
    list->length++;
    
  }
  // 1048576 is how many bytes are in a megabyte
  printf("mb2_mmap: %d MB available\n", (int) (memory_size / 1048576));

  return true;
}

