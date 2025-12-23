#ifndef ISR_H
#include <stdint.h>

void setup_idt();
void load_idt();

struct cpu_context_t {
  uint64_t rax;
  uint64_t rbx; 
  uint64_t rcx; 
  uint64_t rdx;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t r8; 
  uint64_t r9; 
  uint64_t r10; 
  uint64_t r11; 
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  
  uint64_t vec;
  uint64_t error_code;
 
  uint64_t rip_i;
  uint64_t cs_i;
  uint64_t flags_i;
  uint64_t rsp_i;
  uint64_t ss_i;
};

#endif // !ISR_H
