.code64
.text
.global interrupt_handler
.global start_isr_stubs

.include "src/include/context_switch.h"
.macro nocode_stub vec
  .align 16
  pushq $0
  pushq $\vec
   
  jmp isr_stub
.endm

.macro code_stub vec
  .align 16
  pushq $\vec
  jmp isr_stub
.endm


start_isr_stubs:

# Defining all of the exceptions
nocode_stub 0x0
nocode_stub 0x1
nocode_stub 0x2
nocode_stub 0x3
nocode_stub 0x4
nocode_stub 0x5
nocode_stub 0x6
nocode_stub 0x7

# TODO: Change this interrupt to being a code interrupt when timer interrupt vec is changed
nocode_stub 0x8
nocode_stub 0x9
code_stub 0xa
code_stub 0xb
code_stub 0xc
code_stub 0xd
code_stub 0xe
nocode_stub 0xf
nocode_stub 0x10
code_stub 0x11
nocode_stub 0x12
nocode_stub 0x13
nocode_stub 0x14  
code_stub 0x15
nocode_stub 0x16
nocode_stub 0x17
nocode_stub 0x18
nocode_stub 0x19
nocode_stub 0x1a
nocode_stub 0x1b
nocode_stub 0x1c
code_stub 0x1d
code_stub 0x1e
nocode_stub 0x1f


# Defining interrupts with that push a dummy code
.set i, 8
.rept 247
  nocode_stub i
  .set i, i+1
.endr

isr_stub:
  push_regs
  
  # Putting pointer to cpu_context_t struct in %rdi
  mov %rsp, %rdi
  call interrupt_handler
  mov %rax, %rsp

  pop_regs
  add $0x10, %rsp
  
  iretq

