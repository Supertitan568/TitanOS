#include "isr.h"
#include "kernel_heap_manager.h"
#include "scheduler.h"
#include "vga.h"
#include "vmm.h"
#include "mem.h"
#include "vmm_hashmap.h"
#include <libtitan.h>
#include <gdt64.h>
#include <syscall.h>

void idle_main();



process_t* idle_proc = NULL;
process_t* head_proc = NULL;
process_t* current_process = NULL;
thread_t* current_thread = NULL;

extern uintptr_t* current_pml4t_ptr;
extern vmm_hashmap_t vmm_hashmap;


static void kernel_thread_wrapper(void (*thread_start)(void*), void* arg){
  thread_start(arg);
  current_thread->status = DEAD;
  while(1){
    continue;
  }
}

static void user_thread_wrapper(void (*thread_start)(void*), void* arg){
  thread_start(arg);

  // Exit
  execute_syscall(60, 0, 0, 0);  
  idle_main(); 
}

thread_t* create_thread(process_t* p, char name[], void* func, void* func_arg, bool is_user){
  static size_t next_tid;
  thread_t* t = kmalloc(sizeof(thread_t));
  memset(t, 0, sizeof(thread_t)); 

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
  
  //Support multiple user thread stacks per process
  void* stack = is_user ? alloc_user_stack((void*) USER_STACK_START, &p->vmm)  : alloc_kernel_stack();

  t->stack = stack;
  t->parent = p;
  
  // Creating the thread context
  cpu_context_t new_context = {
    .rax = 0,
    .rbx = 0,
    .rcx = 0,
    .rdx = 0,
    .rdi = (uint64_t) func,
    .rsi = (uint64_t) func_arg,
    .rbp = 0,
    .r8 = 0,
    .r9 = 0,
    .r10 = 0,
    .r11 = 0,
    .r12 = 0,
    .r13 = 0,
    .r14 = 0,
    .r15 = 0,
    .vec = 0,
    .error_code = 0,
    .rip_i = (uint64_t) kernel_thread_wrapper,
    .cs_i = is_user ? USER_CODE_OFFSET : CODE_OFFSET,
    .flags_i = 0x00000202,
    .rsp_i = (uint64_t) stack + KERNEL_STACK_SIZE - 0x8,
    .ss_i = is_user ? USER_DATA_OFFSET : DATA_OFFSET
  };
  t->context = new_context;
  
  return t;
}

process_t* create_proc(void* func_point, vmm_t vmm_instance, bool add_to_queue, bool is_user){
  static size_t next_pid = 0;
  
  // We are allocating all of the processes on the heap
  process_t* new_proc = kmalloc(sizeof(process_t));
  memset(new_proc, 0, sizeof(process_t)); 
  // Creating process status
  new_proc->pid = next_pid++; 
  
  new_proc->vmm = vmm_instance;

  // Creating inital thread
  create_thread(new_proc, "thread", func_point, NULL, is_user);


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
  while(true){
    continue;
  }
}

void print1(){
  for(int i = 0; i < 5; i++){
    for(int i = 0; i < 10; i++)
      printstr("Printing 1\n");
  }
}

void print2(){
  const char filename[] = "/file2.txt";
  char buf[20];
  
  // We first open the file
  int file_handle = execute_syscall(2, (uint64_t) filename, 0, 0);

  // We then read 10 chars from the file 
  execute_syscall(0, file_handle, (uint64_t) buf, 10);

  buf[10] = '\0';

  execute_syscall(1,(uint64_t) buf, 0, 0);
}





static void switch_process(){
  if(head_proc != NULL){
    if(current_process->next == NULL){
      current_process = head_proc;
    }
    else{
      current_process = current_process->next;
    }
  }
  else{
    current_process = idle_proc;
  }
  

  switch_vm_space(&current_process->vmm);
}

void reap_thread(thread_t* t);

void reap_process(process_t* p){
  process_t* curr_p = head_proc;
  if(curr_p == p){
    head_proc = curr_p->next;
  }
  else{
    while(curr_p->next != p){
      curr_p = curr_p->next; 
    }

    curr_p->next = curr_p->next->next;
  }
  

  // We kill all threads that may or may not be READY or RUNNING
  thread_t* t = p->threads;
  thread_t* t_to_reap;
  while(t != NULL){
    t_to_reap = t;
    t = t->next;
    reap_thread(t_to_reap);
  }
  
  // We free everything not in kernel space
  vm_object* vm_space = &p->vmm.head;
  vm_object* vms_to_free;
  while(vm_space->base != KERNEL_TEXT_START){
    vms_to_free = vm_space;
    vm_space = vm_space->next;
    vmm_free((void*)vms_to_free->base);
  }
  
  switch_vm_space(&idle_proc->vmm);
  vmm_hashmap_remove(vmm_hashmap, p->vmm.pml4t);
  kfree(p);
}

void reap_thread(thread_t* t){
  vmm_free(t->stack);

  thread_t* current_t = t->parent->threads;
  
  // Should never happen because that would make this a dangling thread that is not tracked by its parent 
  ASSERT(current_t != NULL);
  
  if(current_t == t){
    t->parent->threads = current_t->next;  
  }
  else{
    while(current_t->next != t){
      current_t = current_t->next; 
    }
    
    current_t->next = current_t->next->next;
  }
  
  if(t->parent->threads == NULL){
    reap_process(t->parent);
  }

  kfree(t);
}

cpu_context_t* schedule(cpu_context_t* context){
  if(current_thread != idle_proc->threads){
    current_thread->context = *context;
  }

  thread_t* prev_t = current_thread; 
  if(prev_t->status == DEAD){
    reap_thread(prev_t);  
  }
  else{
    prev_t->status = READY;
  }

  // Switching the thread
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
  idle_proc = create_proc(idle_main, create_vmm(initial_vmm), false, false); 
  current_process = idle_proc;
  current_thread = current_process->threads;
  create_proc(print2, create_vmm(initial_vmm), true, true);
}
