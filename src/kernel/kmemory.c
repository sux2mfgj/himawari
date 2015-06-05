#include "kernel.h"
#include "kmemory.h"
#include "paging.h"

bool init_kmemory(const multiboot_info* mb_info)
{

    // TODO: set below variable
    uintptr_t memory_limit = 0xffffffff;

    printk("flag 1: 0x%x", 0x1 & mb_info->flags);
    printk("flag 2: 0x%x", 0x2 & mb_info->flags);
    printk("flag 4: 0x%x", 0x4 & mb_info->flags);


    uintptr_t aligned_kernel_end = (1 + ((uintptr_t)&_kernel_end >> 4)) << 4;
    printk("kernel end addr before alloc kvmemory: 0x%x", aligned_kernel_end);

    if ((aligned_kernel_end + KVMEMORY_SIZE) < memory_limit) {
        kernel_panic("don't enough memory.");
        return false;
    }
    base_addr = aligned_kernel_end;

    for (int i = 0; i < (KVMEMORY_SIZE / MIN_KVMEMORY_ALLOC_SIZE); ++i) {
        kvmem_list[i].number_of_same_state = 0;
        kvmem_list[i].flag = FREE;
    }

    kvmem_list[0].number_of_same_state =
        (KVMEMORY_SIZE / MIN_KVMEMORY_ALLOC_SIZE);

    return true;
}

//return is aligned MIN_KVMEMORY_ALLOC_SIZE;
void* kmalloc(uint32_t size)
{
    for (int i = 0; i < (KVMEMORY_SIZE / MIN_KVMEMORY_ALLOC_SIZE); ++i) {
        if (ALLOCATED == kvmem_list[i].flag) {
            continue;
        }

        int sequence_free_size =
            kvmem_list[i].number_of_same_state * MIN_KVMEMORY_ALLOC_SIZE;

        // alloc!!
        if (sequence_free_size >= size) {
            int number_of_element = size / MIN_KVMEMORY_ALLOC_SIZE;
            int rest = size % MIN_KVMEMORY_ALLOC_SIZE;

            if (0 != rest) {
                number_of_element++;
            }

            for (int j = 0; j < number_of_element; ++j) {
                kvmem_list[i + j].flag = ALLOCATED;
/*                 kvmem_list[i + j].number_of_same_state = 0; */
            }
            kvmem_list[i + number_of_element].number_of_same_state =
                kvmem_list[i].number_of_same_state - number_of_element;

            kvmem_list[i].number_of_same_state = number_of_element;

            return (void*)(base_addr + MIN_KVMEMORY_ALLOC_SIZE * i);
        }

        i += kvmem_list[i].number_of_same_state - 1;
    }

    return NULL;
}
