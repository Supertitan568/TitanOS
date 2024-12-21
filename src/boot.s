.code16
.org 0
.text
.global _start
  
_start:
  cli
  mov %cs, %ax
  mov %ax, %es
  mov %ax, %ds
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  # Setting up the stack
  mov $0x3000, %sp

  sti

  mov $success, %ebx
  call _print_string

  mov $0x7e00, %bx
  mov $0x30, %dh
  call _read_sectors
  

  # Prepping for protected mode
  lgdt (gdt_descriptor)
   
  cli
  mov %cr0, %eax
  or $0x01, %al
  mov %eax, %cr0
   
  
   # Making the jump to 32 bit land!!!
  ljmp $0x8, $_start_32

.code32
_start_32:
  # Resetting all of the segment registers
  mov $(gdt_data_segment - gdt), %ax
  mov %ax, %es
  mov %ax, %ds
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  # Starting out fresh with a new stack
  mov $0x00090000, %esp
  mov %esp, %ebp
  #Jumping to 32bit stage 2
  mov $0x00007e00, %eax
  jmpl *%eax
  

  
.code16

_hang:
  jmp _hang

_print_string:
  # Parameters: %bx contains address of string
  pusha
  mov $0x0e, %ah
  
_print_string_loop:
  # Repeats until hits null character at the end
  mov (%ebx), %cl
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


  movb $0x00, (%di)
  movb $0xff, (%si)
  
  cmpb $0xff, (%di)
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
  
disabled: .string "The a20 line is disabled\n"
success: .string "Let's get into it boys\n"
error: .string "We failed...\n"

.include "src/gdt.s"

.fill 510-(.-_start), 1, 0 
.word 0xaa55
