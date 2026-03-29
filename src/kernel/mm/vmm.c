#include "mem.h"
#include "pmm.h"
#include "vmm.h"
#include "mem.h"
#include "vmm_hashmap.h"
#include "kernel_heap_manager.h"
#include "vga.h"
#include <libtitan.h>
#include <scheduler.h>

uintptr_t* current_pml4t_ptr = (uintptr_t*) INITIAL_PML4T_ADDR;

extern vmm_hashmap_t vmm_hashmap;
extern process_t* current_process;

extern char initial_pdpt[];  
extern char initial_pd[];  
extern char initial_pt[];
extern char initial_pdpt2[];
extern char initial_pd2[]; 
extern char initial_pt2[];
extern char initial_pt3[];

extern char initial_pml4t_phys[];
extern char initial_pdpt_phys[]; 
extern char initial_pd_phys[]; 
extern char initial_pt_phys[];
extern char initial_pdpt2_phys[]; 
extern char initial_pd2_phys[]; 
extern char initial_pt2_phys[];
extern char initial_pt3_phys[];

// My god I am using hella linked lists. This is more than I will probably ever use again.
// This is the inital linked list for the kernel
vm_object kernel_page_tbls_obj = {
  .base = KERNEL_PAGE_TBL_START,
  .length = 0x6000,
  .flags = VM_FLAG_READ_WRITE,
  .next = NULL
};


vm_object kernel_pmm_obj = {
  .base = KERNEL_PMM_START,
  .length = 0x1000,
  .flags = VM_FLAG_READ_WRITE,
  .next = &kernel_page_tbls_obj
};

vm_object vga_text_buffer_obj = {
  .base = VGA_TEXT_BUFFER_BASE,
  .length = 0x1000,
  .flags = VM_FLAG_MMIO,
  .next = &kernel_pmm_obj 
};

vm_object kernel_text_obj = {
  .base = KERNEL_TEXT_START,
  .length = 0x13000,   // This has to given at runtime
  .flags = VM_FLAG_READ_WRITE,
  .next = &vga_text_buffer_obj
};

vmm_t* current_vmm;

static inline uint16_t get_index(uintptr_t addr, uint8_t table_level){
  // This gets the index of the block in the page table we are looking at
  return (( addr >> (9 * table_level + 12)) & 0x1ff);
}


static inline uintptr_t get_block_vaddr(uintptr_t table_virt_addr, uint16_t table_index, uint8_t table_level){
  // This gets the begining address of what the block controls
  return table_virt_addr + (table_index << (9 * table_level  + 12)); 
}

static uintptr_t get_current_pml4t_phys_addr(){
  uintptr_t current_pml4t;
  asm("mov %%cr3, %0;" : "=r" (current_pml4t));
  return current_pml4t;
}


void switch_vm_space(vmm_t* vmm){
  asm volatile("mov %0, %%cr3" :: "r" (vmm->pml4t) : "memory");
  current_pml4t_ptr = (uintptr_t*) get_ptbl_vaddr(vmm_hashmap,(void*) vmm->pml4t);
  current_vmm = vmm;
}


void umap(uint64_t* table_ptr, uintptr_t table_virt_addr, uint8_t table_level, uintptr_t region_start_addr, uintptr_t region_end_addr){
  // start and end indexes of the region we are looking at
  uint16_t start_index = get_index(region_start_addr, table_level);
  uint16_t end_index = get_index(region_start_addr, table_level);

  for(uint16_t i = start_index; i < end_index; i++){
    // basically if the table is higher than a pt
    uintptr_t pt_phys_addr = (uintptr_t) PAGE_ALIGN_DOWN(table_ptr[i]);

    if(table_level){
      // Addresses of the current block 
      uintptr_t block_start_addr = get_block_vaddr(table_virt_addr, i, table_level);
      uintptr_t block_end_addr = get_block_vaddr(table_virt_addr, i + 1, table_level);

      // This is the bounds of the region for the sub block we are looking at
      uintptr_t start_addr = region_start_addr;
      uintptr_t end_addr = region_end_addr;
      
      // We need to see if the bounds we are looking at in the main function are in the sub block
      if(region_start_addr <= block_start_addr){
        start_addr = block_start_addr;
      }
      if(region_end_addr >= block_end_addr){
        start_addr = block_start_addr;
      }
      // Doing this same thing for the sub block
      umap(get_ptbl_vaddr(vmm_hashmap, (void*) pt_phys_addr), get_block_vaddr(table_virt_addr, i, table_level), table_level - 1, start_addr, end_addr);

      // If the entire block is in the region we need to free it 
      if(region_start_addr <= block_start_addr && region_end_addr >= block_end_addr){
        vmm_hashmap_remove(vmm_hashmap, (void*)PAGE_ALIGN_DOWN(table_ptr[i]));
        table_ptr[i] = 0;
      }
    }
    // This is for when we are at the bottom level pt. We are finally at the lowest region we can find
    else {
      // Clear the entry
      pmm_free_page((void*) pt_phys_addr);
      table_ptr[i] = 0;
    }
  }
}

bool mmap(uint64_t* root_table, void* paddr, void* vaddr, size_t flags){
  // TODO: invalidate pages in the TLB
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
      if(! (pt_vaddr = get_ptbl_vaddr(vmm_hashmap, pt_paddr))){
        return false;
      }
    }
    else{
      // We need to create the new page table in this case
      
      pt_paddr = alloc_pg_table();

      pt_vaddr[pt_offset] = ((uint64_t) pt_paddr) | 0x7;
      pt_vaddr = get_ptbl_vaddr(vmm_hashmap, pt_paddr); 
      
    }

  }

  // After this we have the bottom pt. We just have to write our entry in
  pt_offset = (((uintptr_t) vaddr) >> 12) & 0x1ff;
  pt_vaddr[pt_offset] = ((uint64_t) paddr) | flags | 0x7;
  return true;
}

bool vmm_lengthen(uintptr_t start_region, int inc){
  // We can only work with pages so the lengths in the vmm obj needs to be page aligned
  inc = PAGE_ALIGN(inc);

  vm_object* curr_vm_obj = &current_vmm->head;
  
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
    size_t flag_mmio = VM_FLAG_MMIO;
    if(curr_vm_obj->flags & flag_mmio){
      // TODO: Add code for handling lengthing mmio stuff 
      // pt_paddr = args;
      // pmm_alloc_specific_page(pt_paddr);
      return false;
    }
    else{
      pt_paddr = pmm_alloc_page(); 
    }

    if(!mmap((void*) current_pml4t_ptr, pt_paddr,(void*) pt_vaddr, curr_vm_obj->flags)){
      return NULL;
    }
  }  
  
  return true;
}

void vmm_free(void* start_region){
  // I'm going to assume for right now that this exists in here and it has been allocated with kmalloc
  // TODO: get rid of those assumptions
  vm_object* curr_vmm_obj = &current_vmm->head;
  if(current_vmm->head.base == (uintptr_t)start_region){
    return;   
  }
  else{
    while(((void*) curr_vmm_obj->next->base) != start_region){
      curr_vmm_obj = curr_vmm_obj->next; 
    }
  }
  
  
  umap((uint64_t*)get_current_pml4t_phys_addr(), (uintptr_t)current_pml4t_ptr, 3, (uintptr_t)start_region, (uintptr_t)start_region + curr_vmm_obj->next->length);
  vm_object* tmp_obj = curr_vmm_obj->next;
  curr_vmm_obj->next = curr_vmm_obj->next->next;
  kfree(tmp_obj);

}

void* vmm_alloc(uintptr_t start_region, size_t length, size_t flags, void* args){
  length = PAGE_ALIGN(length);
  start_region = PAGE_ALIGN_DOWN(start_region);
  args = (void*) PAGE_ALIGN_DOWN((uintptr_t) args);
  // Checks to see if the region requested already exists and if it does then just lengthens it else returns an error
  vm_object* current_vm_object = &current_vmm->head;
  uintptr_t found;
  if((void*) start_region == NULL){
    // Going through the list until we find a space with enough space to put a region with the requested size
    // We are just using first fit here
    current_vm_object = &current_vmm->head;
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
        // Let's check to make sure that we have enough space in this area too
        if((start_region < (current_vm_object->base + current_vm_object->length)) && (start_region + length) < (uintptr_t) current_vm_object->next->base){
          return NULL;
        }
        break;
      } 
      current_vm_object = current_vm_object->next;
    }

    found = start_region;
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
    if(args){
      pt_paddr = args;
      pmm_alloc_specific_page(pt_paddr);
    }
    else{
      pt_paddr = pmm_alloc_page(); 
    }

    if(!mmap((void*) current_pml4t_ptr, pt_paddr,  pt_vaddr, flags)){
      return NULL;
    }
    pt_vaddr += 0x1000;
  }

  return (void*) found;
}


void create_sections(section_t** section_list, size_t length){
  // Get a new page and map it into the new virutal memory space
  code_section_t* code_section;
  for(section_t* section = section_list[0]; section != (*section_list) + length; section++){
    switch(section->type){
      case CODE_SECTION:
        code_section = (code_section_t*) section;
        if(! vmm_alloc((uintptr_t) code_section->start, code_section->length, code_section->flags, NULL)){
          PANIC("vmm", "allocating code section failed"); 
        }
        break;
      case STACK_SECTION:
        if(! vmm_alloc((uintptr_t) section->start, section->length, section->flags, NULL)){
          PANIC("vmm", "allocating stack section failed");
        }
        memset(section->start, 0, section->length);
        break;
      default:
        ERROR("vmm", "unknown section type");
        break;
    }
    
  }
    
}

uintptr_t get_mmio_ptr(void* phys_region_start, size_t length){
  length = PAGE_ALIGN(length);
  static uintptr_t mmio_region = VGA_TEXT_BUFFER_BASE + PAGE_SIZE;

  mmio_region += length;
  return mmio_region - length;
}

void* alloc_mmio(void* phys_region_start, size_t length, size_t extra_flags){
  // VGA buffer is one page long and is the only thing allocated before this runs
  uintptr_t mmio_region = get_mmio_ptr(phys_region_start, length);
  vmm_alloc(mmio_region, length, VM_FLAG_MMIO | extra_flags, phys_region_start);
  return (void*) mmio_region;
}


void* alloc_kernel_stack(){
  static uintptr_t kernel_stack_ptr = KERNEL_STACK_START;
  kernel_stack_ptr -= (KERNEL_STACK_SIZE + 0x1000);
  
  section_t stack_section = {
    .length = KERNEL_STACK_SIZE,
    .start = (void*) kernel_stack_ptr,
    .flags = VM_FLAG_READ_WRITE,
    .type = STACK_SECTION
  };
  
  section_t* sections[] = {
    &stack_section 
  };

  create_sections(sections, 1);

  return (void*) kernel_stack_ptr; 
}


void* alloc_user_stack(void* user_stack, vmm_t* vmm){
  switch_vm_space(vmm);

  section_t stack_section = {
    .length = KERNEL_STACK_SIZE,
    .start = (void*) user_stack,
    .flags = VM_FLAG_READ_WRITE,
    .type = STACK_SECTION
  };
  
  section_t* sections[] = {
    &stack_section 
  };

  create_sections(sections, 1);

  switch_vm_space(&current_process->vmm);
  return (void*) user_stack;
}

static void* create_new_pml4t(){
  // The new pml4t will just be at the end of the current page map space
  void* new_pml4t_paddr = alloc_pg_table();
  uintptr_t* new_pml4t_vaddr = get_ptbl_vaddr(vmm_hashmap, new_pml4t_paddr); 

  // Copy over kernel space from current pml4t into the new one
  *(new_pml4t_vaddr + 511) = *(current_pml4t_ptr + 511);
  return new_pml4t_paddr;
}


vmm_t create_vmm(vmm_t new_vmm){
  return (vmm_t) {
    .head = {
      0x00000000,
      0x00000,
      VM_FLAG_READ_WRITE,
      &kernel_text_obj
    },
    .current_vmm_epoch = new_vmm.current_vmm_epoch,

    .previous_vmm_epoch = new_vmm.previous_vmm_epoch, 

    .pml4t = create_new_pml4t()
  };  
}

vmm_t vmm_init(){
  kernel_text_obj.length = (size_t) KERNEL_SIZE; 
  vmm_hashmap = vmm_hashmap_init();
  vmm_hashmap_put(vmm_hashmap, 1, (void*) initial_pml4t,  (void*) initial_pml4t_phys);
  vmm_hashmap_put(vmm_hashmap, 1, (void*) initial_pdpt,  (void*) initial_pdpt_phys);
  vmm_hashmap_put(vmm_hashmap, 1, (void*) initial_pd,  (void*) initial_pd_phys);
  vmm_hashmap_put(vmm_hashmap, 1, (void*) initial_pt,  (void*) initial_pt_phys);
  
  vmm_hashmap_put(vmm_hashmap, 1,  (void*) initial_pdpt2,  (void*) initial_pdpt2_phys);
  vmm_hashmap_put(vmm_hashmap, 1,  (void*) initial_pd2,  (void*) initial_pd2_phys);
  vmm_hashmap_put(vmm_hashmap, 1,  (void*) initial_pt2,  (void*) initial_pt2_phys);
  vmm_hashmap_put(vmm_hashmap, 1,  (void*) initial_pt3,  (void*) initial_pt3_phys);

  vmm_t new_vmm = {
    {
      0x00000000,
      0x00000,
      VM_FLAG_READ_WRITE,
      &kernel_text_obj
    },
    0,
    0,
    (void*) get_current_pml4t_phys_addr() 
  };

  current_vmm = kmalloc(sizeof(vmm_t));
  *current_vmm = new_vmm;

  return new_vmm;
}
