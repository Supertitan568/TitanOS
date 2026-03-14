ENTRY(_start);
SECTIONS
{
    . = 0x7c00; 
    .boot.text : {
        *(.boot.text)
    }
    .stagetwo.text : {
        *(stagetwo.text)
    }

    .stagethree.text : {
        *(stagethree.text)
        /* There is a problem where if the text is less than 4 bytes
         * away from the section boundry the ALIGN Function - 4 will return a value
         * that is before . is currently. Just force . to be in the next sector and
         * everything will be alright
        */

        . += 4;
        . = ALIGN(512) - 4;
        signature = .;
        LONG(0x00454c42)
        bootloader_end = .;

    }

    .multibootv2_struct.bss (NOLOAD) : {
        *(multibootv2_struct.bss)
    }
    /* Discard common unwanted/unneeded sections */
    /DISCARD/ : {
        *(.comment);  
        *(.note.gnu.property);
    }
}
