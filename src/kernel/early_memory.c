#include <page.h>
#include <init.h>
#include <util.h>
#include <elf.h>
#include <string.h>

uint64_t bitmap[BITMAP_ENTRY_NUM];
uintptr_t allocate_base_addr = 0;
uintptr_t available_end;

bool is_enable = false;

static void change_bits(uintptr_t head, int diff_from_head, int num,
                        bool is_set)
{
    int index = diff_from_head / BIT_NUMBER_OF_BYTE;
    int diff  = diff_from_head % BIT_NUMBER_OF_BYTE;

    uint8_t mask = 1 << diff;
    for (int i = 0; i < num; ++i)
    {
        if (is_set)
        {
            ((uint8_t *)head)[index] |= mask;
        }
        else
        {
            ((uint8_t *)head)[index] &= ~mask;
        }

        if (mask == 0x80)
        {
            index++;
            mask = 1;
            continue;
        }
        mask <<= 1;
    }
}

static void set_bits(uintptr_t head, int diff_from_head, int num)
{
    change_bits(head, diff_from_head, num, true);
}

static void clear_bits(uintptr_t head, int diff_from_head, int num)
{
    change_bits(head, diff_from_head, num, false);
}

// create 0 ~ 16MB bitmap
bool init_early_memory_allocator(struct memory_info *m_info)
{
    // set bit all
    for (int i = 0; i < BITMAP_ENTRY_NUM; ++i)
    {
        bitmap[i] = ~0x0UL;
    }

    // setup by boot time memory map
    for (int i = 0; i < PAGE_INFO_MAX; ++i)
    {
        struct page_info pinfo = m_info->pages_info[i];
        if (pinfo.type != MEMORY_USABLE)
        {
            continue;
        }

        if ((pinfo.head % PAGE_SIZE) != 0)
        {
            // panic("should be rounded to PAGE_SIZE");
            continue;
        }

        if (pinfo.length == 0)
        {
            continue;
        }

        //        uintptr_t rounded_head = round_up(pinfo.head, PAGE_SIZE);
        //        if(rounded_head > (pinfo.head + pinfo.length))
        //        {
        //            continue;
        //        }

        int length = pinfo.length;
        if((pinfo.head + length) > EARLY_MEMORY_SIZE)
        {
            length = EARLY_MEMORY_SIZE - pinfo.head;
        }

        clear_bits((uintptr_t)bitmap, pinfo.head / PAGE_SIZE,
                   length / PAGE_SIZE);
    }

    // setup for elf image pages.
    for (int i = 0; i < PAGE_INFO_MAX; i++) {
        struct page_info pinfo = m_info->pages_info[i];
        if(pinfo.type != MEMORY_MODULE)
        {
            continue;
        }

        struct elf_header *header = (struct elf_header *)pinfo.head;
        
        // checking is elf format?
        if(0x7f != header->e_ident[0])
        {
            continue;
        }
        char elf[3];
        for(int i=0; i<3; ++i)
        {
            elf[i] = header->e_ident[1 + i];
        }
        elf[3] = '\0';

        if(!strcmp("elf", elf)){
            continue;
        }

        //WIP
        int elf_size = header->e_shoff + (header->e_shentsize * header->e_shnum); 
        int page_num = elf_size / PAGE_SIZE;

        set_bits((uintptr_t)bitmap, (uintptr_t)header / PAGE_SIZE, page_num);
    }


    // reserved kernel code
    uintptr_t kernel_start_addr = PHYSICAL_START;
    uintptr_t kernel_end_addr =
        round_up(m_info->kernel_end, PAGE_SIZE) - START_KERNEL_MAP;

    if (kernel_end_addr > EARLY_MEMORY_SIZE)
    {
        panic("kernel is too big!!");
    }

    int kernel_code_size = kernel_end_addr - kernel_start_addr;
    available_end        = round_down(m_info->available_end, PAGE_SIZE);

    if (available_end < EARLY_MEMORY_SIZE)
    {
        panic("memory need at least 16MB");
    }

    set_bits((uintptr_t)bitmap, (kernel_start_addr / PAGE_SIZE),
             kernel_code_size / PAGE_SIZE);

    // set bit for NULL(0)
    set_bits((uintptr_t)bitmap, 0, 1);

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

    for (int i = 0; i < BITMAP_ENTRY_NUM; ++i)
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
    panic("early memory not enough");
    return 0;
}
