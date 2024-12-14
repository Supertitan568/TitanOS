.code16

.align 16
gdt_descriptor:
  .word gdt_end - gdt - 1
  .long gdt + 0x7c00


.align 16
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
  .byte 0b10011010
  .byte 0b11001111
  .byte 0x00

gdt_end:


# idt:
#   .word 0
#   .long 0
