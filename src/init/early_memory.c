#include <page.h>
#include <init.h>
#include <util.h>

#define bitmap_size ((EARLY_MEMORY_PAGE_NUM + sizeof(uint64_t) + 1) >> 6)
uint64_t bitmap[bitmap_size];
uintptr_t allocate_base_addr = 0;

bool init_early_memory_allocator(uintptr_t kernel_end_addr, uintptr_t available_end)
{
    uintptr_t round_kernel_end = round_up(kernel_end_addr, PAGE_SIZE);
    uintptr_t round_available_end = round_down(available_end, PAGE_SIZE);

    allocate_base_addr = round_kernel_end;

    //clear bitmap
    for(int i=0; i<bitmap_size; ++i)
    {
        bitmap[i] = 0;
    }

    return true;
}

uintptr_t early_malloc(uintmax_t page_num)
{
    //TODO
    // alloc sequence page
    if(page_num != 1) {
        return 0;
    }

    for(int i=0; i<bitmap_size; ++i)
    {
        if(bitmap[i] != 0xffffffffffffffff) 
        {
            uint64_t mask = 0xf000000000000000;
            for(int j=0; j<64; ++j, mask >>= 1)
            {
                if(!(bitmap[i] & mask))
                {
                    bitmap[i] |= mask;
                    return allocate_base_addr + (i * 64 * PAGE_SIZE) + (j * PAGE_SIZE) + START_KERNEL_MAP;
                }
            }
        }
    }

    // not found
    // TODO: occured panic
    //  panic("early memory not enough");
    return 0;
}
