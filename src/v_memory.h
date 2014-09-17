#ifndef _INCLUDED_V_MEMORY2_H_
#define _INCLUDED_V_MEMORY2_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "k_memory.h"
#include "p_memory.h"
#include "func.h"

#define PTE_PRESENT 0x01
#define PTE_ABSENT 0x00
#define PTE_RW_USER 0x02
#define PTE_RW_SUPERVISOR 0x00
#define PTE_US_USER 0x04
#define PTE_US_SUPERVISOR 0x00
#define PTE_ACCESS 0x20
#define PTE_DIRTY 0x40

#define PAGE_DIRECTORY_TABLE_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define PHYSICAL_KERNEL_ADDR 0x00100000
#define VIRTUAL_KERNEL_ADDR 0xc0000000

bool init_v_memory();

bool create_page_directory(uint32_t* dir_table, uint32_t dir_table_entry_flags,
                           uint32_t page_table_entry_flags);
static uint32_t create_table_entry(uintptr_t base_addr, uint32_t flags);
void map_page(uint32_t* directory_table, uintptr_t physical_addr,
              uintptr_t vitrual_addr);
uint32_t* get_physical_addr(uint32_t* directory_table, uintptr_t virtual_addr);

#endif

