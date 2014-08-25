#include "p_memory.h"

static p_memory_data p_mem_data;

bool init_p_memory(uintptr_t kernel_end_include_heap, uintptr_t memory_end)
{
    /*     printf(TEXT_MODE_SCREEN_LEFT, "p_memory management head addr: 0x%x",
     * kernel_end_include_heap+(4-kernel_end_include_heap%4)); */

    p_mem_data.head_addr = kernel_end_include_heap -
                           (kernel_end_include_heap % PAGE_SIZE) + PAGE_SIZE;

    /*     uintptr_t memory_end = (mem_upper )* 1024; */

    uintptr_t head = kernel_end_include_heap -
                     (kernel_end_include_heap % PAGE_SIZE) + PAGE_SIZE;
    uintptr_t memory_end_4k = memory_end - (memory_end % PAGE_SIZE);
    size_t management_page_size = memory_end_4k - head;
    uint32_t number_of_page = management_page_size / PAGE_SIZE;

    printf(TEXT_MODE_SCREEN_LEFT, "head: 0x%x", head);
    printf(TEXT_MODE_SCREEN_LEFT, "memory_end: 0x%x", memory_end_4k);
    printf(TEXT_MODE_SCREEN_LEFT, "management_page_size: 0x%x",
           management_page_size);
    printf(TEXT_MODE_SCREEN_LEFT, "number_of_page: %d",
           management_page_size / PAGE_SIZE);
    /*     printf(TEXT_MODE_SCREEN_LEFT, "rest: %d", management_page_size %
     * PAGE_SIZE); */

    p_mem_data.bitmap = (bool *)memory_allocate(sizeof(bool) * number_of_page);
    memset(p_mem_data.bitmap, false, number_of_page);

    p_mem_data.free_num = number_of_page;
    p_mem_data.page_num = number_of_page;

    return true;
}

page* alloc_serial_pages(uint32_t number_of_pages)
{
    if (p_mem_data.free_num < number_of_pages) {
        return NULL;
    } else {
        int serial_num = 0;
        uintptr_t head_addr;

        for (int i = 0; i < p_mem_data.page_num; ++i) {
            if (!p_mem_data.bitmap[i]) {
                if (serial_num == 0) {
                    head_addr =
                        (p_mem_data.head_addr + (PAGE_SIZE * i));
                }

                serial_num++;
                if (serial_num == number_of_pages) {
                    for (int j = serial_num; j >= 0; --j) {
                        p_mem_data.bitmap[i - j] = true;
                    }
                    page* ret = (page*)memory_allocate(sizeof(page));
                    ret->head_addr= head_addr;
                    ret->number = number_of_pages;
                    return ret;
                }
            } else {
                serial_num = 0;
            }
        }

        return NULL;
    }
}

// TODO: alloc largest serial pages
// void *alloc_lagest_serial_pages(uint32_t page_num);

bool free_pages(page* free_page)
{
    uintptr_t addr = p_mem_data.head_addr;
    for (int i = 0; i < p_mem_data.page_num; i++) {
        if((addr + (i * PAGE_SIZE)) == free_page->head_addr){

            for (int j = 0; j < free_page->number; j++) {
                if(!p_mem_data.bitmap[i + j]){
                    p_mem_data.bitmap[i + j] = false;
                }
                else {
                    //kernel panic: page allocater have bug
                    return false;
                }
            }
            p_mem_data.free_num += free_page->number;
            return memory_free(free_page);
        }
    }

    return false;
}

