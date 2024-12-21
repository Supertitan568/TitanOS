OUTPUT_FORMAT("binary")
ENTRY(kernel_start)
SECTIONS {
    .text BLOCK(4K): ALIGN(4K){
        *(.text);
    }

    .data BLOCK(4K): AT(0x9000){
        *(.data);
        *(.rodata); 
    }

    .bss BLOCK(4K): ALIGN(4K){
      *(.bss)
    }
    
    /*Discard common unwanted/unneeded sections*/
    /DISCARD/ : {
        *(.comment);
        *(.note.gnu.property);
    }

    
}
