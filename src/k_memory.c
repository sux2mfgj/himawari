#include "k_memory.h"
#include "func.h"
#include "graphic.h"

static memory_data mem_data;
static uintptr_t kernel_end_include_heap;
static size_t mem_upper;

/* uint32_t memtest( uint32_t start, uint32_t end) */
/* { */
/*     char flg486 = 0; */
/*     uint32_t eflg, cr0, i; */

/*     eflg = io_load_eflags(); */
/*     eflg |= EFLAGS_AC_BIT; */
/*     io_store_eflags(eflg); */
/*     eflg = io_load_eflags(); */
/*     if((eflg & EFLAGS_AC_BIT) != 0){ */
/*         flg486 = 1; */
/*     } */
/*     eflg &= ~EFLAGS_AC_BIT; */
/*     io_store_eflags(eflg); */

/*     if(flg486 != 0){ */
/*         cr0 = load_cr0(); */
/*         cr0 |= CR0_CACHE_DISABLE; */
/*         store_cr0(cr0); */
/*     } */

/*     i = memtest_sub(start, end); */

/*     if(flg486 != 0){ */
/*         cr0 = load_cr0(); */
/*         cr0 &= ~CR0_CACHE_DISABLE; */
/*         store_cr0(cr0); */
/*     } */

/*     return i; */
/* } */

bool init_memory(MULTIBOOT_INFO *multiboot_info)
{

    /*     integer_puts((uint32_t)&_text_start, 17, PUTS_RIGHT); */
    /*     integer_puts((uint32_t)&_kernel_end, 18, PUTS_RIGHT); */
    /*     integer_puts(get_size_of_krnel(), 19, PUTS_RIGHT); */
    /*     integer_puts(memtest(0x00400000, 0xbfffffff) / (1024 * 1024), 20,
     * PUTS_RIGHT); */

    uintptr_t aligned_kernel_end = (1 + (_kernel_end >> 3)) << 3;

    mem_upper = multiboot_info->mem_upper * 1024;
    printk(
        PRINT_PLACE_PHYSI_MEM_SIZE, "Physical MEMORY SIZE: 0x%x",
        (multiboot_info->mem_upper + multiboot_info->mem_lower + 1024) * 1024);
    if (mem_upper > KERNEL_HEAP_END) {
        if (!memory_management_init(KERNEL_HEAP_END - (uintptr_t) & aligned_kernel_end,
                                    (uintptr_t) & aligned_kernel_end)) {
            printf(TEXT_MODE_SCREEN_RIGHT,
                   "error was occured by memory_management_init");
            return false;
        }
    } else {
        printf(TEXT_MODE_SCREEN_RIGHT, "physical memory size is not enough");
        printf(TEXT_MODE_SCREEN_RIGHT, "init_memory cause");
        return false;
    }

    if (!init_p_memory(
            kernel_end_include_heap,
            (multiboot_info->mem_upper + multiboot_info->mem_lower + 1024) *
                1024)) {
        printf(TEXT_MODE_SCREEN_RIGHT, "init_p_memory cause");
        return false;
    }

    if (!init_v_memory()) {
        printf(TEXT_MODE_SCREEN_RIGHT, "init_v_memory cause");
        return false;
    }

    return true;
}

uint32_t get_size_of_kernel()
{
    /*     return (); */
    return (&_kernel_end - &_kernel_start);
}

bool memory_management_init(size_t size, uintptr_t base_addr)
{

    int i;

    for (i = 0; i < MEMORY_MANAGEMENT_DATA_SIZE; ++i) {
        mem_data.data[i].base_addr = 0;
        mem_data.data[i].size = 0;
        mem_data.data[i].status = MEMORY_INFO_STATUS_END;
    }

    mem_data.data[0].base_addr = base_addr;

    kernel_end_include_heap = base_addr + size;  // have to be 0x00a00000
    printf(TEXT_MODE_SCREEN_RIGHT, "kernel_end_include_heap: 0x%x",
           kernel_end_include_heap);

    printk(PRINT_PLACE_FREE_KERNEL_HEAP, "FREE KERNEL HEAP SIZE: 0x%x", size);
    printk(PRINT_PLACE_MAX_KERNEL_HEAP, "MAX KERNEL HEAP SIZE: 0x%x", size);
    mem_data.data[0].size = size;
    mem_data.data[0].status = MEMORY_INFO_STATUS_FREE;

    mem_data.end_point = 1;
    mem_data.nodata_elements_count = 0;
    mem_data.heap_size = size;
    mem_data.free_size = size;

    return true;
}

void *memory_allocate(uint32_t size)
{
    size = (1 + (size >> 3)) << 3;
    for (int i = 0; i < MEMORY_MANAGEMENT_DATA_SIZE; ++i) {
        if (mem_data.data[i].status == MEMORY_INFO_STATUS_END) {
            printf(TEXT_MODE_SCREEN_RIGHT, "memory management array over");
            return NULL;
        }

        if (mem_data.data[i].status == MEMORY_INFO_STATUS_NODATA) {
            continue;
        }

        if (mem_data.data[i].status == MEMORY_INFO_STATUS_USED) {
            continue;
        }

        if (mem_data.data[i].status == MEMORY_INFO_STATUS_FREE) {

            if (mem_data.data[i].size >= size) {

                if (mem_data.end_point <= MEMORY_MANAGEMENT_DATA_SIZE) {
                    // slide data
                    for (int j = mem_data.end_point; j > i; --j) {
                        mem_data.data[j + 1] = mem_data.data[j];
                    }

                    // free data
                    mem_data.data[i + 1].base_addr =
                        mem_data.data[i].base_addr + size;
                    mem_data.data[i + 1].size = mem_data.data[i].size - size;
                    mem_data.data[i + 1].status = MEMORY_INFO_STATUS_FREE;

                    // used data
                    printf(TEXT_MODE_SCREEN_RIGHT, "0x%x", size);
                    mem_data.data[i].size = size;
                    mem_data.data[i].status = MEMORY_INFO_STATUS_USED;

                    mem_data.end_point++;
                    mem_data.free_size -= size;

                    printk(PRINT_PLACE_FREE_KERNEL_HEAP,
                           "FREE KERNEL HEAP SIZE: 0x%x", mem_data.free_size);
                    return (void *)mem_data.data[i].base_addr;
                }
                // memory management count size over
                else {
                    printf(TEXT_MODE_SCREEN_RIGHT,
                           "memory management count size over");
                    return NULL;
                }
            }
            // memory size is not enough
            else {
                printf(TEXT_MODE_SCREEN_RIGHT, "memory size is not enough");
            }
        }
    }

    return NULL;
}

/*
 * u : used
 * f : free
 * c : current
 * h : head
 * t : tail
 */
bool memory_free(void *address)
{

    int current_index = 0, next_index = 0, previous_index = 0;
    memory_info *current_info, *next_info, *previous_info;
    // search index
    /*     if (mem_data->data[0].status != MEMORY_INFO_STATUS_NODATA) { */
    /*         current_index = 0; */
    /*         previous_index = 0; */

    /*     } */
    /*     printf(TEXT_MODE_SCREEN_RIGHT,"end_point: %d", mem_data->end_point);
     */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "previous: %d, current: %d, next: %d",
     */
    /*             previous_index, current_index, next_index); */

    for (int i = 0; i < mem_data.end_point; i++) {

        // before current index find
        if (mem_data.data[i].base_addr == (uintptr_t)address) {
            current_index = i;
            goto find;
        } else if (mem_data.data[i].status != MEMORY_INFO_STATUS_NODATA) {
            previous_index = i;
        }
    }
    /*     printf(TEXT_MODE_SCREEN_LEFT, "not found"); */
    return false;

find:
    for (int j = current_index + 1; j < mem_data.end_point; j++) {
        if (mem_data.data[current_index].status != MEMORY_INFO_STATUS_NODATA) {
            next_index = j;
            break;
        }
    }

    /*     printf(TEXT_MODE_SCREEN_RIGHT, "previous: %d, current: %d, next: %d",
     */
    /*             previous_index, current_index, next_index); */

    current_info = &mem_data.data[current_index];
    next_info = &mem_data.data[next_index];
    previous_info = &mem_data.data[previous_index];
    // current is array head
    if (previous_index == current_index) {

        // h - c - u
        if (next_info->status == MEMORY_INFO_STATUS_USED) {
            current_info->status = MEMORY_INFO_STATUS_FREE;
            /*             printf(TEXT_MODE_SCREEN_LEFT, "h - c - u"); */
            /*             return true; */
            goto exit;
        }
        // h - c - f
        else if (next_info->status == MEMORY_INFO_STATUS_FREE) {
            current_info->size += next_info->size;
            current_info->status = MEMORY_INFO_STATUS_FREE;

            next_info->base_addr = 0;
            next_info->size = 0;
            next_info->status = MEMORY_INFO_STATUS_NODATA;
            mem_data.nodata_elements_count++;
            /*             printf(TEXT_MODE_SCREEN_LEFT, "h - c - f"); */
            /*             return true; */
            goto exit;
        }
    } else {

        if (previous_info->status == MEMORY_INFO_STATUS_FREE) {

            // f - c - f
            if (next_info->status == MEMORY_INFO_STATUS_FREE) {
                previous_info->size += current_info->size + next_info->size;

                current_info->size = 0;
                current_info->base_addr = 0;
                current_info->status = MEMORY_INFO_STATUS_NODATA;

                *next_info = *current_info;

                mem_data.nodata_elements_count += 2;
                /*                 printf(TEXT_MODE_SCREEN_LEFT, "f - c - f");
                 */
                /*                 return true; */
                goto exit;
            }
            // f - c - u
            else {
                previous_info->size += current_info->size;

                current_info->size = 0;
                current_info->base_addr = 0;
                current_info->status = MEMORY_INFO_STATUS_NODATA;

                mem_data.nodata_elements_count++;
                /*                 printf(TEXT_MODE_SCREEN_LEFT, "f - c - u");
                 */
                /*                 return true; */
                goto exit;
            }

        } else {
            // u - c - f
            if (next_info->status == MEMORY_INFO_STATUS_FREE) {
                current_info->size += next_info->size;
                current_info->status = MEMORY_INFO_STATUS_FREE;

                next_info->size = 0;
                next_info->base_addr = 0;
                next_info->status = MEMORY_INFO_STATUS_NODATA;

                mem_data.nodata_elements_count++;
                /*                 printf(TEXT_MODE_SCREEN_LEFT, "u - c - f");
                 */
                /*                 return true; */
                goto exit;
            }
            // u - c - u
            else {
                current_info->status = MEMORY_INFO_STATUS_FREE;
                /*                 printf(TEXT_MODE_SCREEN_LEFT, "u - c - u");
                 */
                /*                 return true; */
                goto exit;
            }
        }
    }

    return false;

exit:
    if (mem_data.nodata_elements_count >= 10) {
        memory_management_array_compaction();
    }
    mem_data.free_size += current_info->size;

    printk(PRINT_PLACE_FREE_KERNEL_HEAP, "FREE KERNEL HEAP SIZE: 0x%x",
           mem_data.free_size);
    return true;
}

void print_array_status(void)
{
    for (int i = 0; i < mem_data.end_point; i++) {
        if (mem_data.data[i].status == MEMORY_INFO_STATUS_FREE) {
            /*             printf(TEXT_MODE_SCREEN_RIGHT, "FREE"); */
            /*             printf("(FREE: size:%d)->", mem_data.data[i].size);
             */
        } else if (mem_data.data[i].status == MEMORY_INFO_STATUS_USED) {
            /*             printf(TEXT_MODE_SCREEN_RIGHT, "USED"); */
            /*             printf("(USED: size:%d)->", mem_data.data[i].size);
             */
        } else if (mem_data.data[i].status == MEMORY_INFO_STATUS_NODATA) {
            /*             printf(TEXT_MODE_SCREEN_RIGHT, "NODATA"); */
            /*             printf("(NODATA)->"); */
        }
        /*         printf(TEXT_MODE_SCREEN_RIGHT, "size: %d, base_addr: %d", */
        /*                 mem_data->data[i].size, mem_data->data[i].base_addr);
         */
    }
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "END"); */
    /*     printf("(END)\n"); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "------------------"); */
}

static void memory_management_array_compaction(void)
{
    uint32_t nodata_index, current_index;
    int i = 0;
    nodata_index = current_index = 0;

    /*     printf("nodata_elements_count: %d\n",
     * mem_data.nodata_elements_count); */
    if (mem_data.nodata_elements_count == 0) {
        return;
    }

    for (int i = 0; i <= mem_data.end_point; ++i) {
        if (mem_data.data[i].status == MEMORY_INFO_STATUS_NODATA) {
            /*             printf("test%d\n", i); */
            for (int j = i + 1; j <= mem_data.end_point; ++j) {
                /*                 printf("j:%d\n", j); */
                if (mem_data.data[j].status == MEMORY_INFO_STATUS_END) {
                    mem_data.data[i] = mem_data.data[j];
                    mem_data.nodata_elements_count = 0;
                    mem_data.end_point = i;
                    return;
                } else if (mem_data.data[j].status !=
                           MEMORY_INFO_STATUS_NODATA) {
                    mem_data.data[i] = mem_data.data[j];
                }
            }
        }
    }
}

void alloc_free_test()
{
    memory_data *t;

    // h - c - u
    t = memory_allocate(sizeof(memory_data));
    print_array_status();
    if (memory_free(t)) {
        /*         printf(TEXT_MODE_SCREEN_RIGHT, "yes"); */
    }
    print_array_status();
}

