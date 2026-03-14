#include "isr.h"
#include "kernel_heap_manager.h"
#include "scheduler.h"
#include "vga.h"
#include "vmm.h"
#include "vmm_hashmap.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  READY,
  RUNNING,
  DEAD
} status_t;

void idle_main();

typedef struct process_t{
  size_t pid;
  status_t process_status;
  struct cpu_context_t context;
  vmm_t vmm;
  struct process_t* next;
}process_t;

process_t* idle_proc = NULL;
process_t* head_proc = NULL;

static size_t next_pid = 0;
extern uintptr_t* current_pml4t_ptr;
extern vmm_hashmap_t vmm_hashmap;

/*
 * Basically this is a work around to allocate different kernel stacks for processes
 * while I find something better to do. The kernel stacks need to be in kernel space so they case
 * transfer over between different kernel processes
*/



static void tss_init(){
  void* nmi_stack = get_new_kernel_stack_addr();
}


process_t* create_proc(void* func_point, vmm_t vmm_instance, bool add_to_queue){
  // We are allocating all of the processes on the heap
  process_t* new_proc = kmalloc(sizeof(process_t));
  
  // Creating process status
  new_proc->process_status = READY;
  new_proc->pid = next_pid++; 

  // Allocating stack
  // TODO: Figure out a way to do this in userspace whenever we get there 
  void* kernel_stack = get_new_kernel_stack_addr();
  create_sections((void*) kernel_stack);

  // Creating the process context
  struct cpu_context_t new_context = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (uint64_t) func_point,
    0x8,
    0x00000202,
    (uint64_t) kernel_stack + KERNEL_STACK_SIZE - 0x8,
    0x10
  };
   
  new_proc->context = new_context;

  // If the only process in here is the idle process we should obviously just point the only process to itself
  
  new_proc->vmm = vmm_instance;
  if(add_to_queue){
    if(head_proc == NULL){
      new_proc->next = NULL;
    }
    else{
      new_proc->next = head_proc;
    }
     
    head_proc = new_proc;
  } 
  
  return new_proc;
}

void idle_main(){
  while(true)
    continue;
}

void print1(){
  while(1){
    for(int i = 0; i < 10; i++)
      printstr("Printing 1\n");
  }
}

void print2(){
  while(1){
    printstr("Printing 2\n");
  }
}

process_t* current_process = NULL;

static void ready_process(){
  void* new_pml4t_paddr = current_process->vmm.pml4t;
  asm("mov %0, %%cr3" : "=r" (new_pml4t_paddr));
  current_pml4t_ptr = (uintptr_t*) get_ptbl_vaddr(vmm_hashmap, new_pml4t_paddr); 
  
  // Might be useful when enabling smp
  if(current_process->process_status == READY){
    current_process->process_status = RUNNING;
  }
}


/*
 * This picks out either the next process or the idle process
 * depending on what's in the process queue. It will also clean up any 
 * dead processes if it comes across them
*/

struct cpu_context_t* schedule(struct cpu_context_t* context){
  current_process->context = *context;
  current_process->process_status = READY;
  if(current_process != idle_proc){
    if(current_process->next == NULL){
      current_process = head_proc;
    }
    else{
      current_process = current_process->next;
    }
  }
  else{
    if(head_proc != NULL){
      current_process = head_proc;
    }
    else{
      current_process = idle_proc;
    }
  }
  ready_process();

  return &current_process->context;
}


void sched_init(vmm_t initial_vmm){
  idle_proc = create_proc(idle_main, create_vmm(initial_vmm), false); 
  current_process = idle_proc;
  create_proc(print1, create_vmm(initial_vmm), true);
  create_proc(print2, create_vmm(initial_vmm), true);
}
