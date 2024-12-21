OUTPUT_FORMAT("binary")
ENTRY(kernel_start)
SECTIONS {
    . = 0x8000
    .text {
        *(.text);
    }

    .data {
        *(.data);
        *(.rodata); 
    }

    .bss {
      *(.bss)
    }
    
    /*Discard common unwanted/unneeded sections*/
    /DISCARD/ : {
        *(.comment);
        *(.note.gnu.property);
    }

    
}
