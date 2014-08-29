#include "v_memory.h"

/* static uint32_t *page_directory_table; */
/* static uint32_t *page_table[PAGE_DIRECTORY_NUM]; */

bool init_v_memory()
{
}

static uintptr_t create_page_table_entry(uintptr_t physical_addr, uint32_t flags)
{
    return (physical_addr & 0xfffff000) | flags;
}

