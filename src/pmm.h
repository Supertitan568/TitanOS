#ifndef PMM_H
#define PMM_H
#include <stddef.h>
#include <stdbool.h>

void pmm_init(size_t num_pages);
void* pmm_alloc_page();
bool pmm_check_page_taken(void* paddr);
bool pmm_alloc_specific_page(void* paddr);
void pmm_free_page(void* paddr);



#endif // !PMM_H
