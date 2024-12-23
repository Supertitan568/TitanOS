#ifndef MEM_H
#include <stddef.h>
#include <stdint.h>

void* mem_cpy(void* dest, void* src, size_t num_bytes);
void* mem_set(void* s, int c, size_t n);

#endif // !MEM_H

