#include "v_memory.h"

static uint32_t kernel_directory_table;

bool init_v_memory()
{
    uint32_t dir_flags, page_flags;
    dir_flags = PTE_RW_SUPERVISOR | PTE_US_SUPERVISOR;
    page_flags = dir_flags;
    if (!create_page_directory(&kernel_directory_table, dir_flags,
                               page_flags)) {
        return false;
    }

    for (uint32_t phy = 0x0, vir = 0x0; phy <= 0x100;
         phy += 0x10, vir += 0x10) {
        printf(TEXT_MODE_SCREEN_RIGHT, "vir: 0x%x, phy: 0x%x",
               phy, (uintptr_t)get_physical_addr(&kernel_directory_table, vir));
    }
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "vir: 0x10000, phy: 0x%x", */
    /*            (uintptr_t)get_physical_addr(&kernel_directory_table,
     * 0x10000)); */

    set_page_directory((uintptr_t) & kernel_directory_table);

    return true;
}

bool create_page_directory(uint32_t* dir_table, uint32_t dir_table_entry_flags,
                           uint32_t page_table_entry_flags)
{
    uint32_t* page_table_head_addr =
        (uint32_t*)memory_allocate_4k(PAGE_TABLE_SIZE);

    if (NULL == page_table_head_addr) {
        return false;
    }

    // set page directry table entry
    for (int i = 0; i < PAGE_DIRECTORY_TABLE_SIZE; ++i) {
        uint32_t* page_head = (uint32_t*)((uintptr_t)page_table_head_addr +
                                          (i * PAGE_TABLE_SIZE));

        dir_table[i] =
            create_table_entry((uintptr_t)page_head, dir_table_entry_flags | PTE_PRESENT);

        // set page table entry
        for (int j = 0; j < PAGE_TABLE_SIZE; ++j) {
            page_head[j] =
                create_table_entry(0x00000000, page_table_entry_flags);
        }
    }

    // map page of kernel 0x00000000 -> 0x00000000
    for (uintptr_t k_phy_addr = 0x00000000, k_vir_addr = 0x00000000;
         k_phy_addr <= KERNEL_HEAP_END;
         k_phy_addr += PAGE_SIZE, k_vir_addr += PAGE_SIZE) {

        map_page(&kernel_directory_table, k_phy_addr, k_vir_addr);
    }

    return true;
}

void map_page(uint32_t* directory_table, uintptr_t physical_addr,
              uintptr_t virtual_addr)
{
    uint32_t directory_table_index = (virtual_addr >> 22);
    uint32_t page_table_index = (virtual_addr >> 12) & 0x3ff;
    uintptr_t offset = virtual_addr & 0x00000fff;
int32_t* page_table =
        (uint32_t*)((uintptr_t)directory_table[directory_table_index] &
                    0xfffff000);

    /*     page_table[page_table_index] |= (physical_addr & 0xfffff000); */
    uint32_t flags = page_table[page_table_index] & 0x00000fff;
    page_table[page_table_index] = (physical_addr & 0xfffff000) | flags | PTE_PRESENT;
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

static uint32_t create_table_entry(uintptr_t base_addr, uint32_t flags)
{
    return ((uintptr_t)base_addr & 0xfffff000) | (flags & 0x00000fff);
}
