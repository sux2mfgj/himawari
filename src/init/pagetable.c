#include <page.h>
#include <init.h>
#include <string.h>

uint64_t *kernel_pml4, *kernel_pdpt, *kernel_pd, *kernel_pt;
uintmax_t kernel_page_directory_num;

uint64_t create_entry(uintptr_t physical_addr, uint64_t flags)
{
    return (physical_addr - START_KERNEL_MAP) | PAGE_PRESENT | flags;
}

bool init_pagetable(uintptr_t rounded_kernel_memory_end)
{
    kernel_pml4 = (uint64_t *)early_malloc(1);
    if (kernel_pml4 == 0)
    {
        return false;
    }
    memset(kernel_pml4, 0, 0x1000);

    kernel_pdpt = (uint64_t *)early_malloc(1);
    if (kernel_pdpt == 0)
    {
        return false;
    }
    memset(kernel_pdpt, 0, 0x1000);

    kernel_pd = (uint64_t *)early_malloc(1);
    if (kernel_pd == 0)
    {
        return false;
    }
    memset(kernel_pd, 0, 0x1000);

    kernel_pml4[511] = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPER |
                       (uintptr_t)kernel_pdpt - START_KERNEL_MAP;

    {
        char buf[32];
        itoa(kernel_pml4[511], buf, 16);
        puts("pml4[511]: ");
        puts(buf);
        puts("\n");
    }

    kernel_pdpt[510] = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPER |
                       (uintptr_t)kernel_pd - START_KERNEL_MAP;
    // 0xffffffff80000000 - 0xffffffffb0000000 (1GB)

    // pd[0] 0 ~ 2MB
    // pd[1] 2MB ~ 4MB
    // ...

    kernel_page_directory_num = rounded_kernel_memory_end / 0x200000;

    for (int j = 0; j <= kernel_page_directory_num; ++j)
    {
        uint64_t *pt = (uint64_t *)early_malloc(1);
        if (pt == 0)
        {
            return 0;
        }
        memset(pt, 0, 0x1000);
        for (int i = 0; i < 512; ++i)
        {
            pt[i] = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPER |
                    ((uintptr_t)i << 12) + (j * 0x200000);
        }

        kernel_pd[j] = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPER |
                       (uintptr_t)pt - START_KERNEL_MAP;
    }

    {
        char buf[32];
        itoa((uintptr_t)kernel_pml4 - START_KERNEL_MAP, buf, 16);
        puts("cr3: ");
        puts(buf);
        puts("\n");
    }

    //  movl %eax, %cr3
    __asm__ volatile(
        "movq %0, %%cr3" ::"r"((uintptr_t)kernel_pml4 - START_KERNEL_MAP)
        : "memory");

    return true;
}

uint64_t *create_pml4(void)
{
    uint64_t *pml4, *pdpt, *pd, *pt;

    pml4 = (uint64_t *)early_malloc(1);
    if (pml4 == 0)
    {
        return 0;
    }
    memset(pml4, 0, 0x1000);

    pdpt = (uint64_t *)early_malloc(1);
    if (pdpt == 0)
    {
        return 0;
    }
    memset(pdpt, 0, 0x1000);

    pd = (uint64_t *)early_malloc(1);
    if (pd == 0)
    {
        return 0;
    }
    memset(pd, 0, 0x1000);

    pml4[511] =
        PAGE_PRESENT | PAGE_READ_WRITE | (uintptr_t)pdpt - START_KERNEL_MAP;

    pdpt[510] =
        PAGE_PRESENT | PAGE_READ_WRITE | (uintptr_t)pd - START_KERNEL_MAP;

    for (int j = 0; j <= kernel_page_directory_num; ++j)
    {
        uint64_t *pt = (uint64_t *)early_malloc(1);
        if (pt == 0)
        {
            return 0;
        }
        memset(pt, 0, 0x1000);
        for (int i = 0; i < 512; ++i)
        {
            pt[i] = PAGE_PRESENT | PAGE_READ_WRITE | 
                    ((uintptr_t)i << 12) + (j * 0x200000);
        }

        kernel_pd[j] = PAGE_PRESENT | PAGE_READ_WRITE | 
                       (uintptr_t)pt - START_KERNEL_MAP;
    }

    return pml4;
}
