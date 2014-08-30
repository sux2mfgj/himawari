#include "v_memory.h"

static uint32_t* kernel_directory_table;

bool init_v_memory()
{
    kernel_directory_table = create_page_directory_table();
    if (NULL == kernel_directory_table) {
        return false;
    }
    set_page_directory((uintptr_t)kernel_directory_table);

    return true;
}

uint32_t* create_page_directory_table()
{
    uint32_t* page_table_head_addr = (uint32_t*)memory_allocate(
        sizeof(uint32_t) * PAGE_DIRECTORY_SIZE * PAGE_TABLE_SIZE);

    if (NULL == page_table_head_addr) {
        return NULL;
    }

    uint32_t* directory_table_addr =
        (uint32_t*)memory_allocate(sizeof(uint32_t) * PAGE_DIRECTORY_SIZE);

    if(NULL == directory_table_addr){
        return NULL;
    }

    for (int i = 0; i < PAGE_DIRECTORY_SIZE; ++i) {
        directory_table_addr[i] = create_page_table_entry(
            (uintptr_t)page_table_head_addr[i*0x1000],
            PTE_PRESENT | PTE_RW_SUPERVISOR | PTE_US_SUPERVISOR);
    }

    for (int pdindex = 0; pdindex < PAGE_DIRECTORY_SIZE; ++pdindex) {
        for (int ptindex = 0; ptindex < PAGE_TABLE_SIZE; ++ptindex) {
            page_table_head_addr[(pdindex * PAGE_DIRECTORY_SIZE) + ptindex] =
                create_page_table_entry(0x00000000, PTE_ABSENT |
                                                        PTE_RW_SUPERVISOR |
                                                        PTE_US_SUPERVISOR);
        }
    }

    // set kernel ( Physical Memory 0x00000000 -> 0x00500000)
    for (uintptr_t k_phy_addr = 0x00000000, k_vir_addr = VIRTUAL_KERNEL_ADDR;
         k_phy_addr <= KERNEL_HEAP_END;
         k_phy_addr += PAGE_SIZE, k_vir_addr += PAGE_SIZE) {
        map_page(page_table_head_addr, k_phy_addr, k_vir_addr,
                 PTE_RW_SUPERVISOR | PTE_US_SUPERVISOR);
    }

    return directory_table_addr;
}

static void map_page(uint32_t* directory_table_addr, uintptr_t physcal_addr,
                     uintptr_t virtual_addr, uint32_t flags)
{
    uint32_t pdindex = (uintptr_t)virtual_addr >> 22;
    uint32_t ptindex = (uintptr_t)virtual_addr >> 12 & 0x03ff;

    uint32_t* page_table =
        (uint32_t*)(uintptr_t)(directory_table_addr[pdindex] & 0xfffff000);
    /*     uint32_t* page_table = directory_table_addr + (0x400 * pdindex); */

    page_table[ptindex] =
        create_page_table_entry(physcal_addr, flags | PTE_PRESENT);
}

static uint32_t create_page_table_entry(uintptr_t physcal_addr, uint32_t flags)
{
    return ((uintptr_t)physcal_addr & 0xfffff000) | (flags & 0xfff);
}

static uintptr_t get_physical_addr(uint32_t* directory_table_addr,
                                   void* virtual_addr)
{
    uint32_t pdindex = (uintptr_t)virtual_addr >> 22;
    uint32_t ptindex = ((uintptr_t)virtual_addr >> 12) & 0x03ff;

    uint32_t* page_table =
        (uint32_t*)(uintptr_t)(directory_table_addr[pdindex] & 0xfffff000);

    return ((page_table[ptindex] & ~0xfff) + ((uintptr_t)virtual_addr & 0xfff));
}

