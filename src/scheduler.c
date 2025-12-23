#include "isr.h"
#include "kernel_heap_manager.h"
#include "vmm.h"
#include "hpet.h"
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
  status_t process_status;
  struct cpu_context_t context;
  struct process_t* next;
}process_t;

process_t* idle_proc = NULL;
process_t* head_proc = NULL;

process_t* create_proc(void* func_point){
  // We are allocating all of the processes on the heap
  process_t* new_proc = kmalloc(sizeof(process_t));
  
  // Creating process status
  new_proc->process_status = READY;
  
  // Creating the process context
  //void* stack_ptr = vmm_alloc((uintptr_t) NULL, 0x1000, VM_FLAG_READ_WRITE | VM_FLAG_USER, 0);
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
    (uint64_t) func_point,
    0x8,
    0,
    0, //(uint64_t) stack_ptr,
    0x10
  };
  
  new_proc->context = new_context;

  // If the only process in here is the idle process we should obviously just point the only process to itself

  if(head_proc == NULL){
    new_proc->next = NULL;
  }
  else{
    new_proc->next = head_proc;
  }
  
  head_proc = new_proc;
  return new_proc;
}

static void copy_cpu_context_params(struct cpu_context_t* curr_context, struct cpu_context_t* next_context){
  next_context->vec = curr_context->vec;
  next_context->error_code = curr_context->error_code;
  next_context->cs_i = curr_context->cs_i;
  next_context->flags_i = curr_context->flags_i;
  next_context->rsp_i = curr_context->rsp_i;
  next_context->ss_i = curr_context->ss_i;
} 

void idle_main(){
  while(true)
    continue;
}

process_t* current_process = NULL;


struct cpu_context_t schedule(struct cpu_context_t* context){
  if(current_process != NULL && current_process != idle_proc){
    current_process->context = *context;
    if(current_process->next == NULL){
      current_process = head_proc;
    }
    else{
      current_process = current_process->next;
    }
  }
  else{
    if(head_proc != NULL && head_proc != idle_proc){
      current_process = head_proc;
    }
    else{
      current_process = idle_proc;
    }
  }

  // We need to keep some params from the current cpu_context so iretq doesnt fail
  copy_cpu_context_params(context, &current_process->context);

  return current_process->context;
}

void sched_init(){
  idle_proc = create_proc(idle_main);
  // hpet_init();

  head_proc = NULL;
}
