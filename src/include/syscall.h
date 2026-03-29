#ifndef SYSCALL_H
#define SYSCALL_H

#include <isr.h>
#include <libtitan.h>

cpu_context_t* syscall_handler(cpu_context_t* context);

static inline size_t execute_syscall(size_t syscall_num, size_t arg){
  asm("int $0xfe" : "=a"(arg) : "a"(syscall_num), "D"(arg) : "memory");
  return arg;
}


void syscall_init();
#endif // !DEBUG

