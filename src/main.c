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

    puts("hello", PUTS_LEFT);
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
    char *a = (char *)memory_allocate(memory_data, sizeof(char)*5);
    if (a == 0) {
        puts("memory allocate error", PUTS_RIGHT);
        return;
    }

    a[0] = 't';
    a[1] = 'e';
    a[2] = 's';
    a[3] = 't';
    a[4] = '\0';
    puts(a, PUTS_RIGHT);

    memory_free(memory_data, a);
}

