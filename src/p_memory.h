#ifndef _INCLUDED_P_MEMORY_H_
#define _INCLUDED_P_MEMORY_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "graphic.h"
#include "k_memory.h"
#include "string.h"

#define PAGE_SIZE 4096

typedef struct {
    uintptr_t head_addr;
    uint32_t free_num;
    uint32_t page_num;
    bool* bitmap;
} p_memory_data;

typedef struct  {
    uintptr_t head_addr;
    size_t number;
} page;

bool init_p_memory(uintptr_t kernel_end_include_heap, uintptr_t memory_end);
page* alloc_serial_pages(uint32_t number_of_pages);
// void *alloc_lagest_serial_pages(uint32_t page_num);
bool free_pages(page* free_page);

#endif
