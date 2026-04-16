#ifndef SYSCALL_H
#define SYSCALL_H

#include <isr.h>
#include <libtitan.h>

cpu_context_t* syscall_handler(cpu_context_t* context);

static inline uint64_t execute_syscall(uint64_t syscall_num, uint64_t arg0, uint64_t arg1, uint64_t arg2){
  asm("int $0xfe" : "=a"(arg0) : "a"(syscall_num), "D"(arg0) , "S"(arg1), "d"(arg2): "memory");
  return arg0;
}


void syscall_init();
#endif // !DEBUG

