.code32
.text

_start_stage_two:
   
  mov $0x000b8000, %ebx
  mov $0x0f48, %ax
  mov %ax, (%ebx)
  mov $0, %ecx
  mov (%ebx), %ecx
  cmp %eax, %ecx 
  je _hang

  
_hang:
  jmp _hang

# _print_string:
#   # Parameters: %bx contains address of string
#   pusha
#   mov $0x0e, %ah
#   
# _print_string_loop:
#   # Repeats until hits null character at the end
#   mov %es:(%ebx), %cl
#   cmp $0x00, %cl 
#   je _print_string_end
#   mov %cl, %al
#   int $0x10
#
#   inc %ebx
#   jmp _print_string_loop
#
# _print_string_end:
#   popa
#   ret
#

hello: .string "Helloasdf from 32bit land!!\r\n"
