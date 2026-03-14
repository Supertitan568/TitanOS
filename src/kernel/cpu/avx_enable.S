.code64
.text

# From osdev wiki
enable_avx:
  push %rax
  push %rcx
  push %rdx

  xor %rcx, %rcx
  xgetbv #Load XCR0 register
  or $7, %eax  #Set AVX, SSE, X87 bits
  xsetbv #Save back to XCR0

  pop %rdx
  pop %rcx
  pop %rax
  ret 
