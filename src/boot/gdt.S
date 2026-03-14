.code16

.global gdt_data_offset
gdt_data_offset: .long gdt_data_segment - gdt 

.align 16

.global gdt_descriptor
gdt_descriptor:
  .word gdt_end - gdt_start - 1
  .long gdt_start 


.align 16
gdt_start:

gdt:
  # First NULL descriptor
  .quad 0

gdt_code_segment:
  .word 0xffff
  .word 0x0000
  .byte 0x00
  .byte 0b10011010
  .byte 0b11001111
  .byte 0x00

gdt_data_segment:
.word 0xffff
  .word 0x0000
  .byte 0x00
  .byte 0b10010010
  .byte 0b11001111
  .byte 0x00

gdt_end:


# idt:
#   .word 0
#   .long 0
