.code16
.org 0x0
.section .boot.text, "ax"

# Constants ( Mostly for memory locations )
.set TOP_OF_STACK_16_BIT, 0x7bf8
.set SECTOR_SIZE, 512

# Symbol Imports
.extern _parse_multiboot_header

.extern bootloader_end
.extern signature

.global _start
_start:
  cli
  
  # Like the grub source said we are not safe here
  # We need to reset cs so we know we have all of our segment registers zerod out
  ljmp $0x0, $_start_after_cs     

_start_after_cs:
  # This was causing me mad problems on real hardware
  xor %ax, %ax
  mov %ax, %es
  mov %ax, %ds
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss
  # Setting up the stack
  mov $TOP_OF_STACK_16_BIT, %sp
  mov %sp, %bp

  sti
  
  #mov $bootloader_end, %di
  #call _print_short 
  
  # On every single x86-64 machine I've ran this on A20 has been enabled by default
  # I am not going to waste my time and memory space in the MBR enabling this 
  call _test_a20_line

_get_bootloader:
  
  call _check_lba

  mov $1, %ax
  mov $(_start + SECTOR_SIZE), %bx
  
  # We want to get all of the bootloader sectors + 1 more for the kernel multiboot header
  mov $bootloader_end, %di
  sub %bx, %di
  add $SECTOR_SIZE, %di  
  call _read_bytes
  
  mov $signature, %ebx
  call _print_string
  
  jmp _parse_multiboot_header

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
  # Printing new line char
  mov $0x0d, %al
  int $0x10
  mov $0x0a, %al
  int $0x10

  popa
  ret


_check_lba:
  pusha
  mov $0x41, %ah
  mov $0x55aa, %bx
  mov $0x80, %dx
  int $0x13
  jc _print_error
  popa
  ret

  
# %ax: starting disk addr in sectors
# %bx: starting memory addr
# %di: number of bytes
# %dl: drive letter

.global _read_bytes 
_read_bytes:
  pusha
 
  call _bytes_to_sectors
  push %di                  # We have to save this so we can tell if the bios actually copied the amount of sectors we need

  movw %di, (disk_transfer_size)

  mov %bx, %di
  call _addr_to_seg_reg_value
  
  # Right now this just divides the memory address by 0x10 and shoves it into the segment register
  # This limits us to 64K for any read off the disk 
  # TODO: Break this read into chunks for requests larger than 64K

  movw %di, (disk_memory_addr + 2)
  movw %ax, (disk_disk_addr)
  
  # Setting up interrupt args
  mov $0x42, %ah                  # interrupt 13 function "read lba"
  mov $disk_address_packet, %si   # Disk Packet 
  mov $0x80, %dx
  int $0x13      # %ebx will have pointer to space where this will put it
  #jc _print_error

  # Checking to see if amount of sectors read is the same
  pop %di
  mov (disk_transfer_size), %ax
  cmp %ax, %di
  jne _print_error
  
  # Going back
  popa
  ret


.global _get_bootloader_size 
# %di: will hold the returned size
_get_bootloader_size:
  # I wish I could just have a macro with two symbols
  # Sadly that does not work because of the padding from the linker script
  # That means we have to take the linker symbol and subtract it from the start
  push %ax
  push %bx
  
  mov $_start, %ax
  mov $bootloader_end, %bx
  sub %ax, %bx
  mov %bx, %di 
  
  call _bytes_to_sectors

  pop %bx
  pop %ax

  ret


_addr_to_seg_reg_value:
  push %ax
  push %cx
  push %dx
  
  mov $0x0000, %dx
  mov %di, %ax
  movw $0x0010, %cx
  div %cx
  mov %ax, %di

  pop %dx
  pop %cx
  pop %ax

  ret


# %di: number of bytes (Calling)
# %di: number of sectors (Returning)

_bytes_to_sectors:
  push %ax
  push %cx
  push %dx
  
  mov $0x0000, %dx
  mov %di, %ax
  movw $SECTOR_SIZE, %cx
  div %cx
  
  test %dx, %dx
  jz _bytes_to_sectors_end
  inc %ax

_bytes_to_sectors_end:
  mov %ax, %di
  pop %dx
  pop %cx
  pop %ax

  ret

.global _print_error
_print_error:
  mov $error, %ebx
  call _print_string
  jmp _hang


_test_a20_line:
  pusha 
  push %es
  push %ds
 
  cli
  xor %ax, %ax
  mov %ax, %es
  
  not %ax
  mov %ax, %ds
  
  mov $0x500, %di
  mov $0x510, %si


  movb $0x00, %es:(%di)
  movb $0xff, %ds:(%si)
  
  cmpb $0xff, %es:(%di)
  jne _test_a20_line_exit
  
  push %es
  push %ds
  popa 
  mov disabled, %ebx
  call _print_string
  ret

_test_a20_line_exit:
  pop %es
  pop %ds
  popa 
  ret

#_print_short:
  # Prints whatever is in %ax
  # Need this mostly for debugging
#  push %cx
#  push %ax
#  push %dx

#  mov $4, %cx

  #Printing "0x"
#  mov $0x0e, %ah
#  mov $'0', %al
#  int $0x10
#  mov $'x', %al
#  int $0x10
#_print_short_loop_start:
  # Setting up the right amount of shifts
#  mov %cx, %ax
#  sub $1, %ax
#  mov $4, %bx
#  mul %bx
  # %ax should have 4(%cx - 1) in it
 # push %cx
#  mov %ax, %cx
#  mov %di, %ax
#  shr %cl, %ax
#  pop %cx

#  mov $0x0e, %ah
#  and $0x0f, %al
#  cmpb $0x0a, %al
#  jge _print_short_loop_else
#  add $0x30, %al
#  jmp _print_short_loop_end
#_print_short_loop_else:
#  add $0x51, %al
#_print_short_loop_end:
#  int $0x10
#  loop _print_short_loop_start

  # Printing new line char
#  mov $0x0d, %al
#  int $0x10
#  mov $0x0a, %al
#  int $0x10
#  pop %dx
#  pop %ax
#  pop %cx
#  ret



disk_address_packet:
  .byte 0x10
.byte 0x00 
disk_transfer_size:
  .word 0x0000
disk_memory_addr:
  .long 0x00000000
disk_disk_addr:
  .long 0x00000000
  .long 0x0

disabled: .string "The a20 line is disabled"
error: .string "Error"


.fill 510-(.-_start), 1, 0 
.word 0xaa55
