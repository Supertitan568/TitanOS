#ifndef KERNEL_HEAP_MANAGER_H

#include <stddef.h>

void kheap_init();
void kfree(void* ptr);
void* kmalloc(size_t size);

#endif // !KERNEL_HEAP_MANAGER_H

#define KERNEL_HEAP_MANAGER_H
