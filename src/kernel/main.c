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

//TODO delete below variable
#define startup_processes_num 5
struct task_struct startup_processes[startup_processes_num];

void panic(char *text)
{
    while (true)
    {
        hlt();
    }
}

bool parse_bootinfo(uintptr_t bootinfo_addr, struct memory_info *m_info)
{
    struct multiboot_tag *tag;
    int page_info_index = 0;
    m_info->available_end = 0;
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

                            m_info->pages_info[page_info_index].type =
                                MEMORY_USABLE;
                            m_info->pages_info[page_info_index].head =
                                mmap->addr;
                            m_info->pages_info[page_info_index].length =
                                mmap->len;
                            page_info_index++;

                            m_info->available_end =
                                max(m_info->available_end,
                                    (uintptr_t)(mmap->addr + mmap->len));

                            break;
                        }

                        case MULTIBOOT_MEMORY_RESERVED:
                        {
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
                struct multiboot_tag_module *module =
                    (struct multiboot_tag_module *)tag;

                m_info->pages_info[page_info_index].type   = MEMORY_MODULE;
                m_info->pages_info[page_info_index].head   = module->mod_start;
                m_info->pages_info[page_info_index].length = module->mod_end;

                memset(m_info->pages_info[page_info_index].name, 0,
                       MODULE_NAME_SIZE);
                for (int i = 0; i < MODULE_NAME_SIZE; ++i)
                {
                    char c = *(char *)((uintptr_t) & (module->cmdline) + i);

                    m_info->pages_info[page_info_index].name[i] = c;
                    if (c == '\0')
                    {
                        break;
                    }
                }

                page_info_index++;

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

    status = init_pagetable(round_up(m_info.kernel_end - START_KERNEL_MAP, PAGE_SIZE));
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

    status = init_scheduler();
    if (!status)
    {
        panic("init_scheduler");
    }

    for (int i = 0, j = 0; i < BOOT_MODULES_NUM; ++i, ++j)
    {
        for (; j < PAGE_INFO_MAX; ++j)
        {
            if (m_info.pages_info[j].type == MEMORY_MODULE)
            {
                break;
            }
        }
        if (j == PAGE_INFO_MAX)
        {
            panic("boot module is not enough!");
        }

        status = setup_server_process(
            m_info.pages_info[j].head + START_KERNEL_MAP, &startup_processes[i],
            m_info.pages_info[j].name);

        if (!status)
        {
            panic("setup_server_process");
        }
        add_task(&startup_processes[i]);

    }

    status = init_syscall();
    if (!status)
    {
        panic("init_syscall");
    }

    

    start_task();
    panic("aiee ...after start_task");

    //    sti();

    //    puts("kernel thread");
    //    while (true)
    //    {
    //        hlt();
    //    }
}
