#include <syscall.h>
#include <vga.h>
#include <scheduler.h>

//TODO: Implement syscall and sysret instructions

extern thread_t* current_thread;

cpu_context_t* syscall_handler(cpu_context_t* context){
  switch(context->rax){
    // write  
    case 1:
      printc((char) context->rdi); 
      break;
    // exit 
    case 60:
      current_thread->status = DEAD;

    default:
      PANIC("syscall", "Unknown Syscall");
  }

  return context;
}

void syscall_init(){
}
