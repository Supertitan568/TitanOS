#include <syscall.h>
#include <vga.h>
#include <scheduler.h>
#include <vfs.h>

//TODO: Implement syscall and sysret instructions

extern thread_t* current_thread;

cpu_context_t* syscall_handler(cpu_context_t* context){
  resource_t* current_resource;
  switch(context->rax){
    // read
    case 0:
      current_resource = current_process->resource_table[context->rdi];
      current_resource->read(current_resource, (void*) context->rsi, context->rdx);
      break;

    // write to console 
    case 1:
      printf("%s", (char*) context->rdi); 
      break;

    // open file
    case 2:
      // rdi will contain a pointer to the filename string
      context->rax = register_file_resource((const char*) context->rdi);
      break;

    // exit 
    case 60:
      current_thread->status = DEAD;
      break;

    default:
      PANIC("syscall", "Unknown Syscall");
  }

  return context;
}

void syscall_init(){
}
