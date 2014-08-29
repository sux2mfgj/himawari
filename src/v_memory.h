#ifndef _INCLUDED_V_MEMORY_H_
#define _INCLUDED_V_MEMORY_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "k_memory.h"
#include "p_memory.h"

#define PTE_PRESENT 0x01
#define PTE_ABSENT 0x00
#define PTE_RW_USER 0x02
#define PTE_RW_SUPERVISOR 0x00
#define PTE_US_USER 0x04
#define PTE_US_SUPERVISOR 0x00
#define PTE_ACCESS 0x20
#define PTE_DIRTY 0x40

#define PAGE_DIRECTORY_NUM 1024

bool init_v_memory();

static uintptr_t create_page_table_entry(uintptr_t physical_addr, uint32_t flags);

#endif
