#include <page.h>
#include <init.h>
#include <string.h>

uint64_t *kernel_pml4, *kernel_pdpt, *kernel_pd, *kernel_pt;

bool init_pagetable(uintptr_t rounded_kernel_memory_end)
{
    kernel_pml4 = (uint64_t*)early_malloc(1);
    if (kernel_pml4 == 0) {
        return false;
    }
    memset(kernel_pml4, 0, 0x1000);

    kernel_pdpt = (uint64_t*)early_malloc(1);
    if (kernel_pdpt == 0) {
        return false;
    }
    memset(kernel_pdpt, 0, 0x1000);

    kernel_pd = (uint64_t*)early_malloc(1);
    if (kernel_pd == 0) {
        return false;
    }
    memset(kernel_pd, 0, 0x1000);

    kernel_pml4[511] = PAGE_PRESENT | PAGE_READ_WRITE |
                       (uintptr_t)kernel_pdpt - START_KERNEL_MAP;

    {
        char buf[32];
        itoa(kernel_pml4[511], buf, 16);
        puts("pml4[511]: ");
        puts(buf);
        puts("\n");
    }

    kernel_pdpt[510] = PAGE_PRESENT | PAGE_READ_WRITE |
                       (uintptr_t)kernel_pd - START_KERNEL_MAP;
    // 0xffffffff80000000 - 0xffffffffb0000000 (1GB)

    {
        char buf[32];
        itoa(kernel_pdpt[510], buf, 16);
        puts("pdpt[510]: ");
        puts(buf);
        puts("\n");
    }

    // pd[0] 0 ~ 2MB
    // pd[1] 2MB ~ 4MB
    // ...

    uintmax_t kernel_page_directory_num = rounded_kernel_memory_end / 0x200000;
    {
        char buf[32];
        itoa(rounded_kernel_memory_end, buf, 16);
        puts("rounded_kernel_memory_end: 0x");
        puts(buf);
        puts("\n");
        puts("pd available siz: ");
        itoa(kernel_page_directory_num, buf, 16);
        puts(buf);
        puts("\n");
    }

    for (int j = 0; j <= kernel_page_directory_num; ++j) {
        uint64_t* pt = (uint64_t*)early_malloc(1);
        if (pt == 0) {
            return false;
        }
        for (int i = 0; i < 512; ++i) {
            pt[i] = 
                PAGE_PRESENT | PAGE_READ_WRITE | 
                ((uintptr_t)i << 12) + (j * 0x200000);
        }

        kernel_pd[j] =
            PAGE_PRESENT | PAGE_READ_WRITE | (uintptr_t)pt - START_KERNEL_MAP;
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
