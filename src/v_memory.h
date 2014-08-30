#ifndef _INCLUDED_V_MEMORY_H_
#define _INCLUDED_V_MEMORY_H_

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

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define PHYSICAL_KERNEL_ADDR 0x00100000
#define VIRTUAL_KERNEL_ADDR 0xc0000000

bool init_v_memory();
uint32_t* create_page_directory_table();
static uint32_t create_page_table_entry(uintptr_t physcal_addr, uint32_t flags);

static void map_page(uint32_t* directory_table_addr, uintptr_t physcal_addr,
                     uintptr_t virtual_addr, uint32_t flags);

static uintptr_t get_physical_addr(uint32_t* directory_table_addr,
                                   void* virtual_addr);

#endif
