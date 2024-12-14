ENTRY(_start);
SECTIONS
{
    .text : {
        *(.text);
    }
    .data : {
        *(.data);
        *(.rodata);
    }

    /* Boot signature */
    

    /* Discard common unwanted/unneeded sections */
    /DISCARD/ : {
        *(.comment);
        *(.note.gnu.property);
    }
}
