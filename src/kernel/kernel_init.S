.code32
.section .init.multiboot, "ax"

.set MB2_MAGIC, 0xe85250d6
.set MB2_ARCH, 0
.set MB2_LENGTH, header_end - header_start
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH + MB2_LENGTH)
.macro map_page_table phys_page_frame, page_table, entry, flags
  push %eax
  push %edx
  push %edi

  mov \entry, %edx
  shl $3, %edx
  movl \phys_page_frame, %eax
  add %edx, %eax
  xor \flags, %eax
  movl %eax, (\page_table) 
  
  pop %edi
  pop %edx
  pop %eax
.endm

.macro map_page phys_page_frame, page, entry, flags
  push %eax
  push %edx
  
  mov \entry, %edx
  shl $3, %edx
  movl \phys_page_frame, %eax
  add %edx, %eax
  xor \flags, %eax
  movl %eax, (\page) 
  
  pop %edx
  pop %eax
.endm



.extern kernel_end
# It seems like everything is aligned right after each other when loaded at 1M
header_start:
  .long MB2_MAGIC
  .long MB2_ARCH
  .long MB2_LENGTH
  .long MB2_CHECKSUM

  # Address Tag
  .align 8
  .short 2 
  .short 0 
  .long 24
  .long header_start  
  .long header_start
  .long kernel_end
  .long bss_end 

  # Entry Address Tag
  .align 8 
  .short 3 
  .short 0
  .long 12
  .long _kernel_init
  
  # Memory Map Request Tag 
  .align 8 
  .short 1
  .short 0 
  .long 12
  .long 6

  # End Tag
  .align 8 
  .short 0
  .short 0
  .long 8
header_end:

.section .bss
.set PAGE_SIZE, 0x1000

.global initial_pml4t
.global initial_pdpt
.global initial_pd
.global initial_pt

.global initial_pml4t2
.global initial_pdpt2
.global initial_pd2
.global initial_pt2

.global initial_pt3

.balign 0x1000
initial_pml4t:  
.space PAGE_SIZE
initial_pdpt:  
.space PAGE_SIZE
initial_pd:  
.space PAGE_SIZE
initial_pt:
.space PAGE_SIZE
initial_pdpt2:  
.space PAGE_SIZE
initial_pd2:  
.space PAGE_SIZE
initial_pt2:
.space PAGE_SIZE
initial_pt3:
.space PAGE_SIZE
stack_top:
.space PAGE_SIZE * 4
stack_bottom:
heap_space:
.space PAGE_SIZE
# TODO: Figure out a way to use the gdt64 from the rest of the kernel
.section .init.text, "ax"
.include "src/boot/gdt64.S" 

.code32

.global _kernel_init
_kernel_init:
  cli
  call _setup_pt
  
  mov $0xc0000080, %ecx
  rdmsr
  xor %edx, %edx
  or $(1 << 8), %eax
  wrmsr
  
  # Putting the pml4t into cr3
  mov $initial_pml4t_phys, %eax
  mov %eax, %cr3
  

  mov %cr0, %eax
  or $(1 << 31), %eax
  mov %eax, %cr0
  
  lgdt (gdt64_desc)
   
  ljmp $0x8, $(_setup_64)

_setup_pt:
  pushal
  push %ebx
  # Mapping the PDPT into the PML4T
  mov $511, %edx
  mov $initial_pml4t_phys, %edi
  mov $0x7, %eax
  mov $initial_pdpt_phys, %ebx 
  call _map_page
   
  # Mapping the PD into the PDPT
  xor %edx, %edx
  mov $initial_pdpt_phys, %edi
  mov $initial_pd_phys, %ebx
  call _map_page
  
  # Mapping the base level page into the PD
  mov $initial_pd_phys, %edi
  mov $initial_pt_phys, %ebx
  call _map_page
  
  # Mapping the second base level page into the PD
  mov $1, %edx
  mov $initial_pt3_phys, %ebx
  call _map_page

  # Mapping the kernel image + mb2_info (hopefully) into the higher half
  mov $0, %edx
  mov $KERNEL_TEXT, %ebx
  mov $initial_pt_phys, %edi
  mov $kernel_page_size, %ecx
  add $1, %ecx
  call _add_pe_protected
  
  pop %ebx
  call _check_mb2_info_mapped

  #Mapping the kernel heap into place  
  mov $0, %edx
  mov $kernel_byte_size, %ebx  #We take the second page frame after the kernel image
  add $PAGE_SIZE, %ebx
  mov $initial_pt3_phys, %edi
  call _map_page
   
  # Mapping the inital pmm bitmap into memory
  mov $0xe8, %edx
  add $0x1000, %ebx
  mov $initial_pt_phys, %edi 
  call _map_page

  # Mapping the vga into the first MMIO page slot
  mov $0xc0, %edx
  mov $initial_pt_phys, %edi
  mov $0x1b, %eax
  mov $0xb8000, %ebx
  call _map_page
  
  
  #TODO: Make sure that the multiboot info struct is actually being mapped into our program properly.
  # Right now I am assuming that it comes after our kernel binary and will be covered by the extra couple pages being added to the kernel size in the linker script 

  # Doing the same with identity mapped pages we have now 
  mov $0, %edx
  mov $initial_pml4t_phys, %edi
  mov $initial_pdpt2_phys, %ebx
  call _map_page
  
  mov $initial_pdpt2_phys, %edi
  mov $initial_pd2_phys, %ebx
  call _map_page

  mov $initial_pd2_phys, %edi
  mov $initial_pt2_phys, %ebx
  call _map_page
 
  # We identity map from the second page to 1M
  mov $PAGE_SIZE, %ebx
  mov $initial_pt2_phys, %edi
  add $0x8, %edi 
  mov $257, %ecx
  call _add_pe_protected

  # Enabling PAE 
  mov %cr4, %eax
  or $(1 << 5), %eax
  mov %eax, %cr4
  
  

  popal
  ret

# %edx : first entry in page table
# %ebx : inital page to be mapped
# %edi : inital page table location
# %ecx : number of pages to map
_add_pe_protected:
  pushal
  mov $0x7, %eax
_add_pe_protected_loop:
  call _map_page
  add $0x1000, %ebx
  inc %edx
  loop _add_pe_protected_loop

  popal
  ret

# %edx : page table entry offset
# %edi : page table location
# %eax : flags
# %ebx : physical page frame
_map_page:
  push %eax
  push %edx
  push %edi
  
  # Getting the offset in bytes and putting that into %edi
  shl $3, %edx
  add %edx, %edi

  # Combining the flags with the physical page frame
  or %ebx , %eax
  
  # Putting the combined entry into the page table
  movl %eax, (%edi) 
  
  pop %edi
  pop %edx
  pop %eax
  
  ret


.set PAGE_MASK, 0xfffff000
# Checks to see if the mb2_info struct is mapped with the kernel or not
# It will map it in if it is not
# %ebx : mb2 struct
_check_mb2_info_mapped:
  pushal 
  push %ebx
  # We first align both the mb2 struct and the end of the bss section down to the nearest page boundry
  andl $PAGE_MASK, %ebx 
  
  mov $bss_end, %ecx
  andl $PAGE_MASK, %ecx 

  # We then check to see if they are on the same page
  cmp %ebx, %ecx
  mov %ebx, %eax
  pop %ebx
  je _check_mb2_info_mapped_end
  
  # In this case, it was not mapped with the kernel
  # The first page after the bss is designated for the mb2 struct
  
  # Make sure the mb2 struct does not cross page boundries
  # Page mb2 struct is stored in -> %eax
  # Memory addr of mb2_info -> %ebx
  
  movl (%ebx), %ecx
  add %ebx, %ecx
  
  andl $PAGE_MASK, %ecx 

  cmp %ecx, %eax
  # TODO: Handle the mb2_struct crossing page boundries better
  jne _hang_kernel

  # Map the mb2 struct into memory 
  mov $kernel_page_size, %edx
  mov $initial_pt_phys, %edi
  mov $0x3, %eax
  mov %ecx, %ebx
  call _map_page

_check_mb2_info_mapped_end:
  popal 
  ret 

_hang_kernel:
  jmp _hang_kernel

.code64
 _setup_64:
  # We have to move the stack pointer 8 bytes before the beginning of the stack
  movabs $(stack_bottom - 0x8), %rsp
  mov %rsp, %rbp
  
  # Just setting the WIP bit real quick
  mov %cr0, %rax
  or $(1 << 16), %rax
  mov %rax, %cr0
  # Gets the virtual address of the mb2_info struct
  mov $0, %rdi
  mov %ebx, %edi
  movabs $KERNEL_TEXT, %rax
  sub %rax, %rdi
  movabs $KERNEL_VIRT, %rax
  add %rax, %rdi

  movabs $kernel_start, %rax
  jmp *%rax 
