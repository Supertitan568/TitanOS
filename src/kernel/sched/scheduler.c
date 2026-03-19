#include "isr.h"
#include "kernel_heap_manager.h"
#include "scheduler.h"
#include "vga.h"
#include "vmm.h"
#include "mem.h"
#include "vmm_hashmap.h"
#include <libtitan.h>

#define NAME_MAX 10
typedef enum {
  READY,
  RUNNING,
  DEAD
} status_t;

void idle_main();

typedef struct thread_t{
  size_t tid;
  cpu_context_t context;
  status_t status;
  char name[NAME_MAX];
  struct thread_t* next;
} thread_t;

typedef struct process_t{
  size_t pid;
  thread_t* threads; 
  vmm_t vmm;
  char name[NAME_MAX];
  struct process_t* next;
}process_t;


process_t* idle_proc = NULL;
process_t* head_proc = NULL;

extern uintptr_t* current_pml4t_ptr;
extern vmm_hashmap_t vmm_hashmap;

static void tss_init(){
  void* nmi_stack = get_new_kernel_stack_addr();
}


thread_t* create_thread(process_t* p, char name[], void* func, void* func_arg){
  static size_t next_tid;
  thread_t* t = kmalloc(sizeof(thread_t));
  if(p->threads){
    thread_t* scan = p->threads;
    for(; scan->next != NULL; scan = scan->next);
    scan->next = t; 
  }
  else{
    p->threads = t;
  }

  memcpy(name, t->name, NAME_MAX);

  t->tid = next_tid++;
  t->status = READY;
  t->next = NULL;
  
  void* kernel_stack = get_new_kernel_stack_addr();
  create_sections((void*) kernel_stack);
  // Creating the thread context
  cpu_context_t new_context = {
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
    (uint64_t) func,
    0x8,
    0x00000202,
    (uint64_t) kernel_stack + KERNEL_STACK_SIZE - 0x8,
    0x10
  };
   
  t->context = new_context;
  
  return t;
}

process_t* create_proc(void* func_point, vmm_t vmm_instance, bool add_to_queue){
  static size_t next_pid = 0;
  
  // We are allocating all of the processes on the heap
  process_t* new_proc = kmalloc(sizeof(process_t));
  
  // Creating process status
  new_proc->pid = next_pid++; 
  
  // Creating inital thread
  create_thread(new_proc, "t1", func_point, NULL);

  new_proc->vmm = vmm_instance;

  // Adding process to queue
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
thread_t* current_thread = NULL;

static void switch_vm_space(){
  void* new_pml4t_paddr = current_process->vmm.pml4t;
  asm("mov %0, %%cr3" : "=r" (new_pml4t_paddr));
  current_pml4t_ptr = (uintptr_t*) get_ptbl_vaddr(vmm_hashmap, new_pml4t_paddr); 
}



static void switch_process(){
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

  switch_vm_space();
}

cpu_context_t* schedule(cpu_context_t* context){
  current_thread->context = *context;
  
  if(current_thread->status != DEAD){
    current_thread->status = READY;
  }

  if(current_thread->next){
    current_thread = current_thread->next;
  }
  else{
    switch_process();
    current_thread = current_process->threads;
  }

  // Might be useful when enabling smp
  if(current_thread->status == READY){
    current_thread->status = RUNNING;
  }

  return &current_thread->context;
}


void sched_init(vmm_t initial_vmm){
  idle_proc = create_proc(idle_main, create_vmm(initial_vmm), false); 
  current_process = idle_proc;
  current_thread = current_process->threads;
  create_proc(print1, create_vmm(initial_vmm), true);
  create_proc(print2, create_vmm(initial_vmm), true);
}
