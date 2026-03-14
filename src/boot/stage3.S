.section .stagethree.text, "ax"

.global kernel_start 
.global kernel_end
.global bss_end

.extern gdt_data_offset
.extern kernel_load_begin 
.extern kernel_load_end
.extern multiboot_info_t

.code32

.global _start_32
_start_32:
  # Resetting all of the segment registers
  mov (gdt_data_offset), %eax
  mov %ax, %es
  mov %ax, %ds
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  # Starting out fresh with a new stack that's high enough so it wont interfere with anything
  mov $0x00090000, %esp
  mov %esp, %ebp
  #Jumping to 32bit stage 
  mov $_start_stage_two, %eax
  jmpl *%eax
   
.align 2
_start_stage_two:
  call _clear_screen
  call _copy_kernel_image
  call _clear_kernel_bss
  call _print_string
  
  mov $multiboot_info_t, %ebx 
  mov $0x36d76289, %eax

  mov (kernel_entry), %ecx
  jmp *%ecx


_copy_kernel_image:
  pushal

  mov (kernel_load_start), %esi
  mov (kernel_load_end), %ecx
  sub %esi, %ecx
  
  mov (kernel_start), %edi

  call _copy_bytes

  popal
  ret

_clear_kernel_bss:
  pushal

  xor %eax, %eax
  mov (kernel_end), %edi
  mov (bss_end), %ecx
  
  sub %edi, %ecx
  call _fill_buffer

  popal
  ret 


# %edi: destination
# %eax: value to fill 
# %ecx: number of bytes
_fill_buffer:
  cld
  rep stosb
  ret

# %esi : source
# %edi : destination
# %ecx : number of bytes
_copy_bytes:
  cld 
  rep movsb
  ret

_clear_screen:
  pushal
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
  popal
  ret


_print_string:
  pushal
  mov $0x000b8000, %ebx
  mov $0x0f, %ah
  mov (kernel_end), %ecx
  sub $4, %ecx

_print_string_loop:
  mov (%ecx), %al
  cmp $0, %al
  je _print_string_end
  mov %ax, (%ebx)
  add $0x2, %ebx
  inc %ecx
  jmp _print_string_loop
_print_string_end:
  popal
  ret

_hang_32:
  jmp _hang_32
