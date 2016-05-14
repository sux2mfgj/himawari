#include <page.h>
#include <init.h>
#include <util.h>

// >> 6 is divide sizeof(uint64_t)
#define bitmap_size ((EARLY_MEMORY_PAGE_NUM + 64 - 1) >> 6)

uint64_t bitmap[bitmap_size];
uintptr_t allocate_base_addr = 0;

bool is_enable = false;

bool init_early_memory_allocator(struct memory_info *m_info)
{
    uintptr_t round_kernel_end = round_up(m_info->kernel_end, PAGE_SIZE);
    uintptr_t round_available_end =
        round_down(m_info->available_end, PAGE_SIZE);

    allocate_base_addr = round_kernel_end - START_KERNEL_MAP;

    m_info->kernel_end_include_heap =
        allocate_base_addr + (EARLY_MEMORY_PAGE_NUM * PAGE_SIZE);
    if (round_available_end <
        (allocate_base_addr + (EARLY_MEMORY_PAGE_NUM * PAGE_SIZE)))
    {
        return false;
    }

    // clear bitmap
    for (int i = 0; i < bitmap_size; ++i)
    {
        bitmap[i] = 0;
    }

    is_enable = true;
    return true;
}

uintptr_t early_malloc(uintmax_t page_num)
{
    if (!is_enable)
    {
        // TODO
        // panic("")
        return 0;
    }

    // TODO
    // alloc sequence page
    if (page_num != 1)
    {
        return 0;
    }

    for (int i = 0; i < bitmap_size; ++i)
    {
        if (bitmap[i] != 0xffffffffffffffff)
        {
            uint64_t mask = 0x0000000000000001;
            for (int j = 0; j < sizeof(uint64_t) * 8; ++j, mask <<= 1)
            {
                if (!(bitmap[i] & mask))
                {
                    bitmap[i] |= mask;

                    return allocate_base_addr + (i * 64 * PAGE_SIZE) +
                           (j * PAGE_SIZE) + START_KERNEL_MAP;
                }
            }
        }
    }

    // not found
    // TODO: occured panic
    //  panic("early memory not enough");
    return 0;
}
