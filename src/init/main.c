#include <boot/multiboot2.h>
#include <page.h>
#include <init.h>
#include <util.h>
#include <x86_64.h>
#include <string.h>
#include <process.h>

#include <stdint.h>
#include <stdbool.h>

void user_function(void)
{
    puts("I am Legend");
    puts("\n");
    while(true){

    }
}

void test_thread(void)
{
    sti();
    while(true){}
}

void test_thread2(void)
{
    while(true){}
}

void start_kernel(uintptr_t bootinfo_addr) 
{

    struct multiboot_tag *tag;

    uintptr_t available_end = 0;
    do{
        tag = (struct multiboot_tag*)(bootinfo_addr + 8);

        uint32_t size = tag->size;
        //alignment size 8
        size = (size + 7) & ~7;

        switch(tag->type) {

            case MULTIBOOT_TAG_TYPE_MMAP:
            {

                multiboot_memory_map_t *mmap;
                for(
                    mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                    (uintptr_t)mmap < (uintptr_t)tag + tag->size;
                    mmap = (multiboot_memory_map_t *)((uintptr_t)mmap 
                        + ((struct multiboot_tag_mmap *)tag)->entry_size))
                {
                    switch(mmap->type) {
                        case MULTIBOOT_MEMORY_AVAILABLE:
                        {
                            puts("available: ");
                            uintptr_t addr = mmap->addr;
                            uint64_t len = mmap->len; 

                            char buf[32];
                            itoa(addr, buf, 16);
                            puts(buf);
                            puts(" - ");
                            itoa(len, buf, 16);
                            puts(buf);
                            puts("\n");


							available_end = max(
                                    available_end, (uintptr_t)(addr + len));

                            break;
                        }

                        case MULTIBOOT_MEMORY_RESERVED:
                        {
                            puts("reserved: ");
                            uintptr_t addr = mmap->addr;
                            uint64_t len = mmap->len; 

                            char buf[32];
                            itoa(addr, buf, 16);
                            puts(buf);
                            puts(" - ");
                            itoa(len, buf, 16);
                            puts(buf);
                            puts("\n");

                            break;
                        }

                        default:
                            break;
                    }
                }

                break;
            }

            default:
                break;
        }
        bootinfo_addr += size;

    } while(tag->type != MULTIBOOT_TAG_TYPE_END) ;

    {
        char buf[32];
        itoa((uintptr_t)&_kernel_start, buf, 16);
        puts("start kenrel addr: 0x");
        puts(buf);
        puts("\n");

        itoa((uintptr_t)&_kernel_end, buf, 16);
        puts("end kenrel addr: 0x");
        puts(buf);
        puts("\n");
    }

    bool status = true;
    uintptr_t kernel_end_include_heap;
    init_early_memory_allocator(
            (uintptr_t)&_kernel_end - START_KERNEL_MAP,
            available_end,
            &kernel_end_include_heap);

    init_pagetable(kernel_end_include_heap);
    init_trap();

/*     status = init_local_apic(); */
/*     if(!status)  */
/*     { */
/*         puts("panic!! at init_local_apic"); */
/*         //panic(); */
/*     } */
    status = init_pic();
    if(!status) {
        //panic("panic!!")
    }

    struct task_struct* first_thread;
    struct task_struct* second_thread;
    struct task_struct* user_task;

    first_thread = (struct task_struct*)early_malloc(1);
    second_thread = (struct task_struct*)(first_thread + (((sizeof(struct task_struct) + sizeof(struct task_struct) - 1) >> 4) << 4));
    user_task = (struct task_struct*)(second_thread + (((sizeof(struct task_struct) + sizeof(struct task_struct) - 1) >> 4) << 4));

    uintptr_t stack_end_addr = early_malloc(1);
    if(stack_end_addr){
        //panic("")  ;
    }
    memset((void *)stack_end_addr, 0, 0x1000);
    status = create_first_thread(
            (uintptr_t)&test_thread,
            stack_end_addr,
            0x1000,
            first_thread);

    stack_end_addr = early_malloc(1);
    if(stack_end_addr){
        //panic("")  ;
    }
    memset((void *)stack_end_addr, 0, 0x1000);
    create_kernel_thread(
            (uintptr_t)&test_thread2,
            stack_end_addr,
            0x1000,
            second_thread);


//    start_kernel_thread(first_thread);
//
    stack_end_addr = early_malloc(1);
    if(stack_end_addr){
        //panic("")  ;
    }
    memset((void *)stack_end_addr, 0, 0x1000);

    extern void user_entry(void);
    create_user_process((uintptr_t)&user_entry, 
            stack_end_addr,
            0x1000,
            user_task);

    init_scheduler(user_task, first_thread);
    //start_task(user_task);

    //don't reach this
    //puts("Aiee?!");
    sti();
    while (true) {
        hlt();
    }
}
