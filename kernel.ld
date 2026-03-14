ENTRY(_kernel_init);

KERNEL_PHYS = 1M;
KERNEL_VIRT = 0xffffff8000000000;


SECTIONS
{
  . = KERNEL_PHYS;
  
  .init.multiboot : {
    *(.init.multiboot)
  } 
  .init.text : {
    *(.init.text) 
  } 
  
  /* Basically we need the LMA to stay at 1M + whatever
   * Then, the VMA has to switch to our higher half address
  */
  . = ALIGN(0x1000);
  KERNEL_TEXT = .;
  KERNEL_OFFSET = KERNEL_VIRT - .;
  . = KERNEL_VIRT;

  .text : AT(ADDR(.text) - KERNEL_OFFSET){
    *(.text.begin)
    *(.text)
    *(.text*)
    *(.ltext*);

  }
  .rodata : AT(ADDR(.rodata) - KERNEL_OFFSET){
    *(.rodata*)
    *(.lrodata.*)
  }
  .data : AT(ADDR(.data) - KERNEL_OFFSET){
    *(.data)
    *(.ldata)
    LONG(0x00534f54);
  }

  kernel_end = . - KERNEL_OFFSET;

  .bss : AT(ADDR(.bss) - KERNEL_OFFSET){
    *(.COMMON)
    *(.bss)
    *(.lbss)
  }
  initial_pml4t_phys = initial_pml4t - KERNEL_OFFSET;
  initial_pdpt_phys = initial_pdpt - KERNEL_OFFSET;
  initial_pd_phys = initial_pd - KERNEL_OFFSET;
  initial_pt_phys = initial_pt - KERNEL_OFFSET;
  
  initial_pdpt2_phys = initial_pdpt2 - KERNEL_OFFSET;
  initial_pd2_phys = initial_pd2 - KERNEL_OFFSET;
  initial_pt2_phys = initial_pt2 - KERNEL_OFFSET;

  initial_pt3_phys = initial_pt3 - KERNEL_OFFSET;

  bss_end = . - KERNEL_OFFSET;
  kernel_page_size = ((bss_end - KERNEL_PHYS) / 0x1000) + 1;
  kernel_byte_size = kernel_page_size * 0x1000;
}
