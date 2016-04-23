#include <boot/multiboot2.h>
#include <page.h>
#include <init.h>
#include <util.h>
#include <x86_64.h>
#include <string.h>
#include <process.h>
#include <kernel.h>

#include <stdint.h>
#include <stdbool.h>

void panic(char *text)
{
    char buf[32];
    puts("panic:");
    puts("\n");
    puts(__FILE__);
    itoa(__LINE__, buf, 10);
    puts(" : ");
    puts(buf);
    puts(" - ");
    puts(text);
    puts("\n");

    while (true)
    {
        hlt();
    }
}

void user_function(void)
{
    puts("I am Legend\n");
    while (true)
    {
    }
}

void test_thread(void)
{
    sti();
    while (true)
    {
    }
}

void test_thread2(void)
{
    while (true)
    {
    }
}

bool parse_bootinfo(uintptr_t bootinfo_addr, struct memory_info *m_info)
{
    struct multiboot_tag *tag;
    int free_page_index = 0;
    do
    {
        tag = (struct multiboot_tag *)(bootinfo_addr + 8);

        uint32_t size = tag->size;
        // alignment size 8
        size = (size + 7) & ~7;

        switch (tag->type)
        {
            case MULTIBOOT_TAG_TYPE_MMAP:
            {
                multiboot_memory_map_t *mmap;
                for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
                     (uintptr_t)mmap < (uintptr_t)tag + tag->size;
                     mmap = (multiboot_memory_map_t
                                 *)((uintptr_t)mmap +
                                    ((struct multiboot_tag_mmap *)tag)
                                        ->entry_size))
                {
                    switch (mmap->type)
                    {
                        case MULTIBOOT_MEMORY_AVAILABLE:
                        {

                            m_info->free_pages[free_page_index].head =
                                mmap->addr;
                            m_info->free_pages[free_page_index].length =
                                mmap->len;
                            free_page_index++;

                            m_info->available_end =
                                max(m_info->available_end,
                                    (uintptr_t)(mmap->addr + mmap->len));

                            break;
                        }

                        case MULTIBOOT_MEMORY_RESERVED:
                        {
                            //
                            break;
                        }

                        default:
                            break;
                    }
                }

                break;
            }
            case MULTIBOOT_TAG_TYPE_MODULE:
            {
                // TODO
                //                struct multiboot_tag_module*
                //                module =
                //                    (struct
                //                    multiboot_tag_module*)tag;

                break;
            }

            default:
                break;
        }
        bootinfo_addr += size;

    } while (tag->type != MULTIBOOT_TAG_TYPE_END);

    return true;
}

void start_kernel(uintptr_t bootinfo_addr)
{
    struct memory_info m_info;
    bool status = true;

    m_info.kernel_end   = (uintptr_t)&_kernel_end;
    m_info.kernel_start = (uintptr_t)&_kernel_start;

    status = parse_bootinfo(bootinfo_addr, &m_info);
    if (!status)
    {
        panic("parse_bootinfo");
    }

    status = init_early_memory_allocator(&m_info);
    if (!status)
    {
        panic("init_early_memory_allocator");
    }

    status = init_pagetable(m_info.kernel_end_include_heap);
    if (!status)
    {
        panic("init_pagetable");
    }

    status = init_trap();
    if (!status)
    {
        panic("init_trap");
    }

    //    status = init_local_apic();
    //    if(!status)
    //    {
    //          panic("init_local_apic");
    //    }

    status = init_pic();
    if (!status)
    {
        panic("init_pic");
    }

    /*     struct task_struct* first_thread; */
    /*     struct task_struct* second_thread; */
    /*     struct task_struct* user_task; */

    /*     first_thread = (struct task_struct*)early_malloc(1); */
    /*     second_thread = (struct task_struct*)(first_thread + */
    /*                                           (((sizeof(struct
     * task_struct) +
     */
    /*                                              sizeof(struct
     * task_struct) -
     * 1) >> */
    /*                                             4) */
    /*                                            << 4)); */
    /*     user_task = (struct task_struct*)(second_thread + */
    /*                                       (((sizeof(struct task_struct) +
     */
    /*                                          sizeof(struct task_struct) -
     * 1)
     * >> */
    /*                                         4) */
    /*                                        << 4)); */

    /*     uintptr_t stack_end_addr = early_malloc(1); */
    /*     if (stack_end_addr) { */
    /*         // panic("")  ; */
    /*     } */
    /*     memset((void*)stack_end_addr, 0, 0x1000); */
    /*     status = create_first_thread((uintptr_t)&test_thread,
     * stack_end_addr,
     */
    /*                                  0x1000, first_thread); */

    /*     stack_end_addr = early_malloc(1); */
    /*     if (stack_end_addr) { */
    /*         // panic("")  ; */
    /*     } */
    /*     memset((void*)stack_end_addr, 0, 0x1000); */
    /*     create_kernel_thread((uintptr_t)&test_thread2, stack_end_addr,
     * 0x1000, */
    /*                          second_thread); */

    /*     //    start_kernel_thread(first_thread); */
    /*     // */
    /*     stack_end_addr = early_malloc(1); */
    /*     if (stack_end_addr) { */
    /*         // panic("")  ; */
    /*     } */
    /*     memset((void*)stack_end_addr, 0, 0x1000); */

    /*     extern void user_entry(void); */
    /*     create_user_process((uintptr_t)&user_entry, stack_end_addr,
     * 0x1000,
     */
    /*                         user_task); */

    /*     init_scheduler(user_task, first_thread); */
    // start_task(user_task);

    // don't reach this
    // puts("Aiee?!");
    sti();
    while (true)
    {
        hlt();
    }
}
