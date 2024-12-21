ENTRY(_start);
SECTIONS
{
    . = 0x7c00; 
    /* .text : { */
    /*     *(.text); */
    /* } */
    /* .data : { */
    /*     *(.data); */
    /*     *(.rodata); */
    /* } */

    /* Boot signature */
    

    /* Discard common unwanted/unneeded sections */
    /DISCARD/ : {
        *(.comment);
        *(.note.gnu.property);
    }
}
