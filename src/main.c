#include"graphic.h"
#include"segment.h"
#include"func.h"
#include"multiboot.h"
#include"memory.h"

void memory_management_test(memory_data* memory_data);

void kernel_entry(unsigned int magic, MULTIBOOT_INFO *multiboot_info)
{

    memory_data memory_data;

    io_cli();
    init_gdtidt();
    init_pit();
    init_pic();
    init_memory(&memory_data);
    io_sti();

    h_puts("hello");
/*     integer_puts(multiboot_info->mem_upper, 21); */
/*     integer_puts(multiboot_info->mmap_addr, 22); */
/*     integer_puts(multiboot_info->mmap_length, 23); */


    memory_management_test(&memory_data);

    for(;;){
        io_hlt();
    }
}


void memory_management_test(memory_data* memory_data)
{
    char *a = (char *)memory_allocate(memory_data, sizeof(char)*3);

    a[0] = 't';
    a[1] = 'e';
    a[2] = '\0';
    h_puts(a);

    memory_free(memory_data, a);
}

