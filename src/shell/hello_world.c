#include <stddef.h>
#include <stdint.h>

static inline uint64_t execute_syscall(uint64_t syscall_num, uint64_t arg0, uint64_t arg1, uint64_t arg2){
  asm("int $0xfe" : "=a"(arg0) : "a"(syscall_num), "D"(arg0) , "S"(arg1), "d"(arg2): "memory");
  return arg0;
}

int main(){
  const char str[] = "Hello from Userspace!";
  
  execute_syscall(1,(uint64_t) str, 0, 0);
}
