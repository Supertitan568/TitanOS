.align 4

gdt64:
  .long 0x00000000
  .long 0x00000000

gdt64_code_segment:
  .word 0xffff 
  .word 0x0000
  .byte 0x00
  .byte 0b10011010
  .byte 0b10101111
  .byte 0x0

gdt64_data_segment:
  .word 0x0000
  .word 0x0000
  .byte 0x00
  .byte 0b10010010
  .byte 0b10100000
  .byte 0x00
gdt64_end:

gdt64_descriptor:
  .word gdt64_code_segment - gdt64 - 1 
  .long gdt64

# .set gdt64_code_ptr, $0x
# .set gdt64_data_ptr, gdt64_data_segment - gdt64_start
