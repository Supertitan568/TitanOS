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
  
  lgdt (gdt64_desc)
  
  ljmp $0x8, $(_setup_64)

.code64 
_setup_64:
  mov $0xffffff8000200000, %rsp
  mov %rsp, %rbp
  
  mov $0xffffff8000000000, %rax
  jmp *%rax 

.code32 
_setup_pt:
  pusha

  # Clearing out 0x1000 memset style
  mov $0xe000, %edi     # Memory address that we are clearing out
  mov %edi, %cr3        # Setting the pml4t pointer to the area we are clearing
  xor %eax, %eax        # Setting eax to 0
  mov $4096, %ecx       # Putting the amount of space we are clearing out
  rep stosl             # Setting 4096 longs to %eax

  mov %cr3, %edi
  
  # This sets the first entry of the pt, pdt, and pdpt
  # These are the temp page tables that we need here in the bootloader on our transition to higher half
  movl $0x2003, (%edi)
  movl $0x2000, %edi
  movl $0x3003, (%edi)
  addl $0x1000, %edi
  movl $0x4003, (%edi)
  
  # I'm only making the page tables identity mapped
  # This will be to 0x6fff
  addl $0x1008, %edi
  mov $0x00001003, %ebx
  mov $7, %ecx
  call _add_pe_protected

  #mov $0x4000, %edi
  #mov $0xb8003, (%edi)
  
  # Imma recursivly map the pml4t to knock that out
  movl $0xeff8, %edi    # PML4T Entries
  movl $0xf003, (%edi)  

  movl $0xf000, %edi    # PDPT Entry
  movl $0x10003, (%edi)

  movl $0x10000,%edi     # PD Entry
  movl $0x11003,(%edi)
 
  # I really could not think of any good place to put more page tables except here
  add $8, %edi
  movl $0x12003, (%edi)
  
  #We gotta map out the first part of this page table to the higher half kernel
  # 0xffffff8000000000 to 0xffffff8000006000
  movl $0x11000, %edi
  movl $0x00008003, %ebx
  movl $6, %ecx
  call _add_pe_protected
  
  # We gotta map out the second part to put in the initial heap space
  # 0xffffff800006000 to  0xffffff80000c000 
  movl $0x11030, %edi
  movl $0x13003, %ebx
  mov  $32, %ecx
  call _add_pe_protected
  
  # Mapping out stack space
  # 0xffffff8000fa000 0xffffff800100000 
  movl $0x11fd0, %edi
  add $0x20000, %ebx
  mov  $6, %ecx
  call _add_pe_protected
  
  # Mapping in page tables for the kernel we are loading
  # 0xffffff800200000 to  0xffffff800206000 
  movl $0x12000, %edi
  mov $0xe003, %ebx
  mov  $5, %ecx
  call _add_pe_protected
 
  # Creating page entry for 0x7fe0000 so we can get the RSDT
  # This will start at the virutal address 0x200000
  #mov $0x5000, %edi
  #mov $0x7fe0013, %ebx
  #mov $512, %ecx
  #call _add_pe_protected
  #
  #mov $0x3008, %edi
  #movl $0x5003, (%edi)
  #
  # Putting a couple entries in for the local/io apic at phys addr 0xfee00000/0xfec00000
  # Will start at 0x400000
  #mov $0x6000, %edi 
  #movl $0xfee00013, (%edi)
  #add $8, %edi
  #movl $0xfec00013, (%edi)
  #
  #mov $0x3010, %edi
  #movl $0x6003, (%edi)
  #
  mov %cr4, %eax
  or $(1 << 5), %eax
  mov %eax, %cr4

  popa
  ret

_add_pe_protected:
  pusha
_add_pe_protected_loop:
  movl %ebx, (%edi)
  add $0x1000, %ebx
  add $8, %edi
  loop _add_pe_protected_loop

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

hello: .string "Hello from 32bit land!!"

.include "src/gdt64.S"
