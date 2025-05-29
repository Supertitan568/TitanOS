. = KERNEL_OFFSET;

ENTRY(kernel_start);

KERNEL_OFFSET = 0xFFFFFF8000000000;

SECTIONS
{
   
  .text : 
  {
    *(.text.begin)
    *(.text)
    *(.text*)
    *(.ltext*);

  }
  .rodata :
  {
    *(.rodata*)
    *(.lrodata.*)
  }
  .data :
  {
    *(.data)
  }
  .bss :
  {
    *(.COMMON)
    *(.bss)
  }

  kernel_end = .;  
}
