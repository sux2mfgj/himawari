#include "v_memory.h"

static uint32_t* kernel_directory_table;

bool init_v_memory()
{
    uint32_t dir_flags, page_flags;
    dir_flags = PTE_RW_SUPERVISOR | PTE_US_SUPERVISOR;
    page_flags = dir_flags;

    kernel_directory_table = create_page_directory(dir_flags, page_flags, true);
    if (NULL == kernel_directory_table) {
        return false;
    }
    printf(TEXT_MODE_SCREEN_RIGHT, "kernel_directory_table addr 0x%x",
           kernel_directory_table);

    /*     printf(TEXT_MODE_SCREEN_RIGHT, "vir: 0x%x, phy: 0x%x", 0x100000, */
    /*            get_physical_addr(kernel_directory_table, 0x100000)); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "vir: 0x%x, phy: 0x%x", 0x400000, */
    /*            get_physical_addr(kernel_directory_table, 0x400000)); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "vir: 0x%x, phy: 0x%x", 0x800000, */
    /*            get_physical_addr(kernel_directory_table, 0x800000)); */
    disable_paging();
    set_page_directory((uintptr_t)kernel_directory_table - 0xc0000000);
    start_4k_paging();
    start_paging();
    /*     enable_paging(); */

    return true;
}

uint32_t* create_page_directory(uint32_t dir_table_entry_flags,
                                uint32_t page_table_entry_flags,
                                bool physical_flag)
{
    uintptr_t diff = 0;
    if(physical_flag){
        diff = 0xc0000000;
    }

    uint32_t* directory_table = (uint32_t*)memory_allocate_4k(1);
    if (NULL == directory_table) {
        return NULL;
    }

    uint32_t* page_table_head_addr =
        (uint32_t*)memory_allocate_4k(PAGE_TABLE_ENTRY_NUM);

    if (NULL == page_table_head_addr) {
        return NULL;
    }
    printf(TEXT_MODE_SCREEN_RIGHT, "page table addr 0x%x",
           page_table_head_addr);

    // set page directry table entry
    for (int i = 0; i < PAGE_DIRECTORY_TABLE_SIZE; ++i) {
        uint32_t* page_head =
            (uint32_t*)((uintptr_t)page_table_head_addr + (i * PAGE_SIZE) - diff);

        directory_table[i] = create_table_entry(
            (uintptr_t)page_head, dir_table_entry_flags | PTE_PRESENT);

        // set page table entry
        for (int j = 0; j < PAGE_TABLE_ENTRY_NUM; ++j) {
            page_head[j] = create_table_entry(
                0x00000000, page_table_entry_flags | PTE_ABSENT);
        }
    }

    // kernel memory space mapping  to virtual memory
    for (uintptr_t k_phy_addr = 0x00000000, k_vir_addr = VIRTUAL_KERNEL_ADDR;
         k_phy_addr <= KERNEL_HEAP_END;
         k_phy_addr += PAGE_SIZE, k_vir_addr += PAGE_SIZE) {

        /*         map_page(directory_table, k_phy_addr, k_vir_addr); */
        map_page(directory_table, k_phy_addr, k_vir_addr);
    }

    return directory_table;
}

static uint32_t create_table_entry(uintptr_t base_addr, uint32_t flags)
{
    return ((uintptr_t)base_addr & 0xfffff000) | (flags & 0x00000fff);
}

void map_page(uint32_t* directory_table, uintptr_t physical_addr,
              uintptr_t virtual_addr)
{
    uint32_t directory_table_index = (virtual_addr >> 22);
    uint32_t page_table_index = (virtual_addr >> 12) & 0x3ff;
    uintptr_t offset = virtual_addr & 0x00000fff;

    uint32_t* page_table =
        (uint32_t*)((uintptr_t)directory_table[directory_table_index] &
                    0xfffff000);

    uint32_t flags = page_table[page_table_index] & 0x00000fff;
    page_table[page_table_index] =
        (physical_addr & 0xfffff000) | flags | PTE_PRESENT;
}


uint32_t* get_physical_addr(uint32_t* directory_table, uintptr_t virtual_addr)
{
    uint32_t directory_table_index = (virtual_addr >> 22);
    uint32_t page_table_index = (virtual_addr >> 12) & 0x000003ff;
    uintptr_t offset = virtual_addr & 0x00000fff;

    uint32_t* page_table_addr =
        (uint32_t*)((uintptr_t)directory_table[directory_table_index] &
                    0xfffff000);

    uint32_t* page_addr =
        (uint32_t*)((uintptr_t)page_table_addr[page_table_index] & 0xfffff000);

    return (uint32_t*)((uintptr_t)page_addr | offset);
}

