#include"memory.h"
#include"func.h"
#include"graphic.h"


unsigned int memtest( unsigned int start, unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0){
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return i;

}

void init_memory(memory_data* memory_data)
{
    integer_puts((unsigned int)&_text_start, 17);
    integer_puts((unsigned int)&_kernel_end, 18);
    integer_puts(get_size_of_kernel(), 19);
    integer_puts(memtest(0x00400000, 0xbfffffff) / (1024 * 1024), 20);


    memory_management_init(memory_data);
}

unsigned int get_size_of_kernel()
{
/*     return (); */
    return (&_kernel_end- &_kernel_start);
}

void memory_management_init(memory_data* memory_data)
{

    int i;

    for(i = 0; i < MEMORY_MANAGEMENT_DATA_SIZE; ++i){
        memory_data->data[i].base_addr = 0;
        memory_data->data[i].size = 0;
        memory_data->data[i].status = MEMORY_INFO_STATUS_NODATA;
    }

    memory_data->data[0].base_addr = &_kernel_end;
    memory_data->data[0].size = 0x00100000;
    memory_data->data[0].status = MEMORY_INFO_STATUS_FREE;

    memory_data->end_point = 1;

}

void* memory_allocate(memory_data* memory_data, unsigned int size)
{
    int i, j;
    for(i = 0; i < MEMORY_MANAGEMENT_DATA_SIZE; ++i){
        if (memory_data->data[i].status == MEMORY_INFO_STATUS_END){
            return (void *)0;
        }

        if (memory_data->data[i].status == MEMORY_INFO_STATUS_NODATA) {
            continue;
        }

        if (memory_data->data[i].status == MEMORY_INFO_STATUS_USED) {
            continue;
        }

        if (memory_data->data[i].status == MEMORY_INFO_STATUS_FREE) {

            if (memory_data->data[i].size >= size) {

                if (memory_data->end_point >= MEMORY_MANAGEMENT_DATA_SIZE) {
                    // slide data
                    for(j = memory_data->end_point; j > i; --j){
                        memory_data->data[j + 1] = memory_data->data[j];
                    }

                    // free data
                    memory_data->data[i + 1].base_addr = memory_data->data[i].base_addr + size;
                    memory_data->data[i + 1].size = memory_data->data[i].size - size;
                    memory_data->data[i + 1].status = MEMORY_INFO_STATUS_FREE;

                    // used data
                    memory_data->data[i].size = size;
                    memory_data->data[i].status = MEMORY_INFO_STATUS_USED;

                    return memory_data->data[i].base_addr;
                }
                // memory management count size over
                else {
                    return (void *)0;
                }
            }
            // memory size is not enough
            else {
                return (void *)0;
            }
        }

    }
    return (void *)0;
}

/*
 * u : used
 * f : free
 * c : current
 * h : head
 * t : tail
 */
unsigned int memory_free(memory_data* memory_data, void *address)
{
    int i;
    for (i = 0; i < memory_data->end_point; i++) {
        if (memory_data->data[i].base_addr == address) {

            // current data is array head
            if (i == 0) {

                // h - c - u
                if (memory_data->data[1].status != MEMORY_INFO_STATUS_FREE) {
                    memory_data->data[0].status = MEMORY_INFO_STATUS_FREE;
                }
                // next status is free
                // h - c - f
                else {
                    memory_data->data[1].base_addr = memory_data->data[0].base_addr;
                    memory_data->data[1].size += memory_data->data[0].size;

                    memory_data->data[0].status = MEMORY_INFO_STATUS_NODATA;
                    memory_data->data[0].base_addr = 0;
                    memory_data->data[0].size = 0;
                }
                return 0;
            }

            // current data is array last
            if (i == MEMORY_MANAGEMENT_DATA_SIZE) {
                //TODO: write current data is array last
                return 0;
            }

            if (memory_data->data[i - 1].status == MEMORY_INFO_STATUS_FREE) {

                //before status is free but next is USED
                // f - c - u
                if (memory_data->data[i + 1].status != MEMORY_INFO_STATUS_FREE) {
                    memory_data->data[i - 1].size += memory_data->data[i].size;

                    memory_data->data[i].base_addr = 0;
                    memory_data->data[i].size = 0;
                    memory_data->data[i].status = MEMORY_INFO_STATUS_NODATA;
                }
                // f - c - f
                else {
                    memory_data->data[i].status = MEMORY_INFO_STATUS_FREE;
                }
            }
            else {
                // u - c - f
                if (memory_data->data[i + 1].status == MEMORY_INFO_STATUS_FREE) {
                    memory_data->data[i].size += memory_data->data[i + 1].size;
                    memory_data->data[i].status = MEMORY_INFO_STATUS_FREE;

                    memory_data->data[i + 1].base_addr = 0;
                    memory_data->data[i + 1].size = 0;
                    memory_data->data[i + 1].status = MEMORY_INFO_STATUS_NODATA;
                }
                // u - c - u
                else {
                    memory_data->data[i].base_addr = 0;
                    memory_data->data[i].size = 0;
                    memory_data->data[i].status = MEMORY_INFO_STATUS_FREE;
                }
            }

            return 0;

        }
    }

    return -1;
}

// TODO: memory_data->data array commpaction
// void memory_management_array_compaction(void){
// }
