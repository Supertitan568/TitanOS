.section .multibootv2_struct.bss

.global multiboot_info_t
.global multiboot_info_t.size
.global multiboot_info_t.reserved
.global multiboot_info_t.memory_tag
.global multiboot_info_t.memory_tag.type
.global multiboot_info_t.memory_tag.size
.global multiboot_info_t.memory_tag.entry_size
.global multiboot_info_t.memory_tag.entry_version
.global multiboot_info_t.memory_tag.entries
.global multiboot_size
.global multiboot_memory_tag_size

.set multiboot_size, multiboot_info_t.memory_tag.entries - multiboot_info_t
.set multiboot_memory_tag_size, multiboot_info_t.memory_tag.entries - multiboot_info_t.memory_tag

multiboot_info_t:
multiboot_info_t.size: .long (multiboot_info_t.memory_tag.entries - multiboot_info_t)
multiboot_info_t.reserved: .space 4

multiboot_info_t.memory_tag:
multiboot_info_t.memory_tag.type: .long 6
multiboot_info_t.memory_tag.size: .long (multiboot_info_t.memory_tag.entries - multiboot_info_t.memory_tag) 
multiboot_info_t.memory_tag.entry_size: .long 24
multiboot_info_t.memory_tag.entry_version: .long 0

multiboot_info_t.memory_tag.entries:


