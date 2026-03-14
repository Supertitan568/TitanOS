.section .stagetwo.text, "ax"

.extern _print_error
.extern _read_bytes
.extern _get_bootloader_size
.extern _start_32

.extern multiboot_info_t
.extern multiboot_info_t.size
.extern multiboot_info_t.reserved

.extern multiboot_info_t.memory_tag
.extern multiboot_info_t.memory_tag.type
.extern multiboot_info_t.memory_tag.size
.extern multiboot_info_t.memory_tag.entry_size
.extern multiboot_info_t.memory_tag.entry_version
.extern multiboot_info_t.memory_tag.entries

.extern multiboot_size
.extern multiboot_memory_tag_size


.global kernel_start
.global kernel_end
.global bss_end
.global kernel_load_start
.global kernel_load_end

.global kernel_entry
.global _parse_multiboot_header

kernel_start: .long 0x00000000
kernel_end: .long 0x00000000
bss_end: .long 0x00000000
kernel_entry: .long 0x00000000
kernel_load_start: .long 0x00000000
kernel_load_end: .long 0x00000000
entry_length: .byte 0x00


.code16
_verify_magic_number:
  #This function is not strictly nessessary but it helps with my sanity
  pusha
  mov (bootloader_end), %bx
  cmpw $0x50d6, %bx
  jne _print_error
  add $2, %bx
  
  mov (bootloader_end + 2), %bx
  cmpw $0xe852, %bx
  jne _print_error

  popa
  ret



_get_section_ends:
  push %ax
  push %bx
  # TODO: Just replace this thing with a loop lol. Much cleaner
  mov $bootloader_end, %bx 
  add $28, %bx
  mov (%bx), %ax 
  mov %ax, (kernel_start)

  add $2, %bx
  mov (%bx), %ax 
  mov %ax, (kernel_start + 2)
 
  add $2, %bx
  mov (%bx), %ax 
  mov %ax, (kernel_end)

  add $2, %bx
  mov (%bx), %ax 
  mov %ax, (kernel_end + 2)
 
  add $2, %bx
  mov (%bx), %ax 
  mov %ax, (bss_end)

  add $2, %bx
  mov (%bx), %ax 
  mov %ax, (bss_end + 2)

  add $10, %bx
  mov (%bx), %ax 
  mov %ax, (kernel_entry)
  
  add $2, %bx
  mov (%bx), %ax 
  mov %ax, (kernel_entry + 2)
 
  pop %bx
  pop %ax
  
  ret

# %edi: memory address to put these entries
_read_e820:
  pusha
  
  xor %ebx, %ebx
_read_e820_loop:
  movl $24, (%edi) 
  add $4, %edi
  
  # We need to store this in entry length so we get the final size
  mov %ebx, (entry_length)
  mov $0x534d4150, %edx
  mov $0x0000e820, %eax
  mov (multiboot_info_t.memory_tag.entry_size), %ecx

  int $0x15
 
  # TODO: Check ACPI: 3.0 Extended attibutes
  jc _read_e820_end
  
  # I simply do not care about the acpi extended field
  # We are just make sure that each entry is 24 byte aligned whether it is 20 or 24 bytes long 
  add $20, %edi

  test %ebx, %ebx
  jnz _read_e820_loop
_read_e820_end:
  # Final entry_length size is length - 1 since we store it at the beginning. We just increment by one to fix that
  incl (entry_length)
  
  mov (entry_length), %al
  mov (multiboot_info_t.memory_tag.entry_size), %bl
  mul %bl

  add %ax, (multiboot_info_t.memory_tag.size)
  add %ax, (multiboot_info_t.size)

  popa
  ret

_fetch_kernel:
  pusha
  
  # %dx:%ax = kernel_end
  mov (kernel_end), %ax
  mov (kernel_end + 2), %dx
  
  # %cx:%bx = kernel_start
  mov (kernel_start), %bx
  mov (kernel_start + 2), %cx
  
  # %dx:%ax = (kernel_end - kernel_start)
  sub %bx, %ax
  sbb %cx, %dx

  test %dx, %dx   # Basically if this if this any besides zero we are def above our 64k limit
  jnz _print_error
  
     
  #TODO: Support a kernel that is over 64K

  # This gets the kernel disk index
  # Bootloader size = Kernel Disk Address  
  mov %ax, %di
  push %di
  call _get_bootloader_size
  mov %di, %ax
  pop %di 
  
  mov $(bootloader_end + 0x1000), %bx
  mov $0x80, %dl
  call _read_bytes
 
  #TODO: Check some signature so we are very sure we got the entire kernel image off
  mov %bx, (kernel_load_start)

  mov %bx, %ax
  mov $0x0000, %dx
  
  mov %di, %bx
  mov $0x0000, %cx
  
  add %bx, %ax
  adc %cx, %dx

  mov %ax, (kernel_load_end)
  mov %dx, (kernel_load_end + 2)
  
  popa
  ret

_multiboot_info_t_init:
  movl $multiboot_size, (multiboot_info_t.size)
  movl $0, multiboot_info_t.reserved

  movl $6, (multiboot_info_t.memory_tag.type)
  movl $multiboot_memory_tag_size, (multiboot_info_t.memory_tag.size)
  movl $24, (multiboot_info_t)
  movl $0, (multiboot_info_t.memory_tag.entry_version)

  ret

_parse_multiboot_header:
  # A proper parser would actually like properly parse the tags and validate the checksum 
  # I do not care to do that I just want to boot only my kernel 
  # Use grub if you want a proper bootloader
  call _verify_magic_number
  call _get_section_ends
  call _multiboot_info_t_init
  call _fetch_kernel 
  call _read_e820

_prep_32:
  # Prepping for protected mode
  cli

  lgdt (gdt_descriptor)
   
  mov %cr0, %eax
  or $0x01, %al
  mov %eax, %cr0
   # Making the jump to 32 bit land!!!
  ljmp $0x8, $_start_32
