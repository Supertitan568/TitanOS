#include "unit_tests.h"
#include "mem.h"
#include "vga.h"
#include "kernel_heap_manager.h"
#include "pmm.h"
#include "vmm.h"
#include "vmm_hashmap.h"

#include <stdint.h>

#define BLOCK_SIZE 8

extern uintptr_t* current_pml4t_ptr;
static inline void assert_true(uintptr_t arg1, uintptr_t arg2){
  if(arg1 != arg2){
     printstr("Assert Failed");
  }
}


// VGA TESTS
static void vga_test1(){
  printf("%s : %s", "Hello There!!", "supertitan");
  printf("%r", "Whatever");
}


static void vga_test2(){
  printf("Hello, %s\n", "supertitan"); 
  printf("NULL Addr: %lu\n", 0);
  printf("Random Addr: %lu\n", 0x123456789);
}


static void vga_test3(){
  for(int i = 0; i < 90; i++){
    printstr("a");
  }
}


// Interrupt tests
static void interrupt_test1(){
  float zero_div = 100 / 0;
}


// Memory Manager tests
static void test_kmalloc(){
  void* ptr = kmalloc(BLOCK_SIZE);
  printstr("First ptr:");
  printlong((uint64_t) ptr);
  printc('\n');

  void* second_ptr = kmalloc(BLOCK_SIZE);
  printstr("Second ptr:");
  printlong((uint64_t) second_ptr);
  printc('\n');

  kfree(ptr);

  ptr = kmalloc(BLOCK_SIZE);
  printstr("Third ptr:");
  printlong((uint64_t) ptr);
  printc('\n'); 
}

static void test_kmalloc2(){
  void* ptr = kmalloc(BLOCK_SIZE);
  printstr("First ptr:");
  printlong((uint64_t) ptr);
  printc('\n');

  void* second_ptr = kmalloc(BLOCK_SIZE);
  printstr("Second ptr:");
  printlong((uint64_t) second_ptr);
  printc('\n');

  void* third_ptr = kmalloc(BLOCK_SIZE);
  printstr("Third ptr:");
  printlong((uint64_t) third_ptr);
  printc('\n');

  kfree(ptr);
  kfree(second_ptr); 
  
  ptr = kmalloc(BLOCK_SIZE+9);
  printstr("Fourth ptr:");
  printlong((uint64_t) ptr);
  printc('\n'); 
}

static void test_pmm_alloc_page(){
  pmm_init(0x9000);
  void* ptr = (void*) 0x0;
  pmm_alloc_specific_page(ptr);
  printstr("First Page Alloc:");
  printlong((uint64_t) ptr);
  printc('\n');

  void* second_ptr = pmm_alloc_page();
  printstr("Second Page Alloc:");
  printlong((uint64_t) second_ptr);
  printc('\n');
  
  void* third_ptr = pmm_alloc_page();
  printstr("Third Page Alloc:");
  printlong((uint64_t) third_ptr);
  printc('\n');
  
  for(int i = 0; i< 20; i++){
    printlong((uint64_t) pmm_alloc_page());
    printc('\n');             
  }
}

// static void test_vmm_hashmap(){
//   pmm_init(0x9000); 
//   vmm_hashmap_init();
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x1000);
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x2000);
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x3000);
//
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x4000);
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x5000);
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x6000);
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x7000);
//
//   vmm_hashmap_put(1, (void*) 0x4000, (void*) 0x8000);
//   vmm_hashmap_put(1, (void*) 0x8000, (void*) 0xf000);
//   vmm_hashmap_put(1, (void*) 0xe000, (void*) 0xc000);
//
//   for(int i = 0; i < 10; i++){
//   }
// }
//
//
// static void test_mmap(){
//   vmm_hashmap_init();
//   vmm_hashmap_put(1, (void*) 0x1000, (void*) 0x1000);
//   vmm_hashmap_put(1, (void*) 0x2000, (void*) 0x2000);
//   vmm_hashmap_put(1, (void*) 0x3000, (void*) 0x3000);
//   vmm_hashmap_put(1, (void*) 0x4000, (void*) 0x4000);
//
//   mmap((void*) current_pml4t_ptr, (void*) 0xb8000, (void*) 0xf000, 0);
//   *((uint8_t*) 0xf000) = (uint8_t) 0x0f48;
// }

static void test_vmm_alloc(){
  vmm_init();
  uint8_t* test_ptr = (uint8_t*) vmm_alloc((uintptr_t) NULL , 0x1000, VM_FLAG_MMIO, (void*) 0xb8000);
  if(test_ptr == NULL){
    printstr("Test failed");
  }
  else{
    *test_ptr = (uint8_t) 0x0f48;
  }
}

void run_unit_tests(){

  vga_test1();
  return;
}

