.code16
.text
.global _start

_start:
  mov $0x7c0, %bx
  mov %bx, %es
  mov %bx, %ds
  # Setting up the stack
  mov $0x3000, %sp
  mov $success, %ebx
  call _print_string
  
  mov $0x200, %bx
  mov $0x30, %dh
  call _read_sectors
  
  # mov $0x00, %ah
  # mov $0x03, %al
  # int $0x10

  # Prepping for protected mode
  # mov $0x7c0, %bx
  # mov %bx, %ds 
  cli
  lgdt gdt_descriptor
  mov %cr0, %eax
  or $0x01, %al
  mov %eax, %cr0
  
   # Making the jump to 32 bit land!!!
  ljmp $0x8, $(_start_32 + 0x7c00)

.code32
_start_32:
  # Resetting all of the segment registers
  mov $(gdt_data_segment - gdt), %ax
  mov %ax, %es
  mov %ax, %ds
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %cs
  mov %ax, %ss
  # Starting out fresh with a new stack
  mov $0x00090000, %esp
  mov %esp, %ebp

  #Jumping to 32bit stage 2
  mov $0x00007e00, %eax
  jmpl *%eax
  

  
_hang:
  jmp _hang
.code16


_print_string:
  # Parameters: %bx contains address of string
  pusha
  mov $0x0e, %ah
  
_print_string_loop:
  # Repeats until hits null character at the end
  mov %es:(%ebx), %cl
  cmp $0x00, %cl 
  je _print_string_end
  mov %cl, %al
  int $0x10

  inc %ebx
  jmp _print_string_loop

_print_string_end:
  popa
  ret


_read_sectors:
  pusha
  push %dx
  
  # Setting up interrupt args
  mov $0x02, %ah # interrupt 13 function "read"
  mov %dh, %al   # No. of sectors 
  
  mov $0x2, %cl  # Starting sector
  mov $0x0, %ch  # cylinder no. 
  mov $0x0, %dh  # head no.

  int $0x13      # %ebx will have pointer to space where this will put it
  jc _print_error
  

  # Checking to see if amount of sectors read is the same
  pop %dx
  cmp %al, %dh
  jne _print_error
  
  # Going back
  popa
  ret

_print_error:
  mov $error, %ebx
  call _print_string
  jmp _hang


_test_a20_line:
  push %ds
  push %es
  push %ax

  mov $0xffff, %ax
  mov %ax, %ds
  
  not %ax
  mov %ax, %es
  
  mov $0x500, %di
  mov $0x510, %si


  movb $0x00, %es:(%di)
  movb $0xff, %es:(%si)
  
  cmpb $0xff, %es:(%di)
  jne _test_a20_line_exit
  pop %ax
  pop %es
  pop %ds
  mov disabled, %ebx
  call _print_string
  ret

_test_a20_line_exit:
  pop %ds
  pop %es
  pop %ax 

  ret
  
disabled: .string "The aaa a20 line is disabled\r\n"
success: .string "Let's get into it boys\r\n"
error: .string "We failed...\r\n"

.include "src/gdt.s"

.fill 510-(.-_start), 1, 0 
.word 0xaa55
