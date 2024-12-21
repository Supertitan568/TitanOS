.code32
.text

_start_stage_two:
  call _clear_screen
  call _print_string
  call _setup_pt

  cli
  mov $0xc0000080, %ecx
  rdmsr
  or $(1 << 8), %eax
  wrmsr
  
  mov %cr0, %eax
  or $(1 << 31), %eax
  mov %eax, %cr0
  
  ljmp $0x8, $(_setup_64 + 0x7c00)

.code64 
_setup_64:
  #lgdt (gdt64_descriptor)
  mov $0x10, %ax
  

  mov $0x0, %rcx
  mov $0x8000, %rax
  mov $0x90000, %rsp
  mov %rsp, %rbp
  jmp *%rax 

.code32 
_setup_pt:
  pusha

  # Clearing out 0x1000 memset style
  mov $0x1000, %edi     # Memory address that we are clearing out
  mov %edi, %cr3        # Setting the pml4t pointer to the area we are clearing
  xor %eax, %eax        # Setting eax to 0
  mov $4096, %ecx       # Putting the amount of space we are clearing out
  rep stosl             # Setting 4096 longs to %eax

  mov %cr3, %edi
  
  # This sets the first entry of the pt, pdt, and pdpt
  movl $0x2003, (%edi)
  addl $0x1000, %edi
  movl $0x3003, (%edi)
  addl $0x1000, %edi
  movl $0x4003, (%edi)

  addl $0x1000, %edi
  mov $0x00000003, %ebx
  mov $512, %ecx

_add_pe_protected:
  movl %ebx, (%edi)
  add $0x1000, %ebx
  add $8, %edi
  loop _add_pe_protected

  mov %cr4, %eax
  or $(1 << 5), %eax
  mov %eax, %cr4

  popa
  ret

_clear_screen:
  pusha
  mov $0x000b8000, %ebx
  mov $0x0020, %ax
  mov $0x000b8000, %ecx
  add $0x7d0, %ecx

_clear_screen_loop:
  cmp %ebx, %ecx
  je _clear_screen_end
  mov %eax, (%ebx)
  add $2, %ebx
  jmp _clear_screen_loop

_clear_screen_end:
  popa
  ret


_print_string:
  pusha
  mov $0x000b8000, %ebx
  mov $0x0f, %ah
  mov $hello, %ecx
  add $0x7c00, %ecx

_print_string_loop:
  mov (%ecx), %al
  cmp $0, %al
  je _print_string_end
  mov %ax, (%ebx)
  add $0x2, %ebx
  inc %ecx
  jmp _print_string_loop
_print_string_end:
  popa
  ret

.include "src/gdt64.s"
hello: .string "Helloasdf from 32bit land!!"

