#include <init.h>
#include <kernel.h>
#include <process.h>
#include <segment.h>
#include <x86_64.h>
#include <string.h>
#include <util.h>
#include <elf.h>

static uint64_t *setup_entry()
{
    uint64_t *entry = (uint64_t *)early_malloc(1);
    if (entry == 0)
    {
        return NULL;
    }
    memset(entry, 0, 0x1000);
    return (uint64_t *)create_entry((uintptr_t)entry,
                                    PAGE_READ_WRITE | PAGE_USER_SUPER);
}

bool setup_server_process(uintptr_t elf_header, struct task_struct *task,
                          char *name)
{
    // should be clear set
    memcpy(task->name, name, 32);
    task->pml4 = create_pml4();
    if (task->pml4 == 0)
    {
        return false;
    }
    task->context.ret_cs = SERVER_CODE_SEGMENT;
    task->context.ret_ss = SERVER_DATA_SEGMENT;

    struct elf_header *header = (struct elf_header *)elf_header;

    struct elf_program_header *p_header =
        (struct elf_program_header *)(elf_header + header->e_phoff);

    for (int i = 0; i < header->e_phnum; ++i)
    {
        uint64_t offset       = p_header[i].p_offset;
        uintptr_t load_v_addr = p_header[i].p_vaddr;
        int size              = p_header[i].p_memsz;

        switch (p_header[i].p_type)
        {
            case PT_LOAD:
                break;
            default:
                continue;
        }

        if (size == 0)
        {
            continue;
        }

        int pml4_index   = load_v_addr >> 39;         // 512G
        int pdpt_index   = load_v_addr >> 30 & 0x1ff; // 1G 0b100000000 512
        int pd_index     = load_v_addr >> 21 & 0x1ff; // 2M
        int pt_index     = load_v_addr >> 12 & 0x1ff; // 4K
        int pt_entry_num = (size + ((1 << 12) - 1)) >> 12;

        if ((uint64_t *)task->pml4[pml4_index] == NULL)
        {
            uint64_t *entry        = setup_entry();
            task->pml4[pml4_index] = (uintptr_t)entry;
        }

        uint64_t **pdpt =
            (uint64_t **)((uintptr_t)(task->pml4[pml4_index] & ~0xfffUL) +
                          START_KERNEL_MAP);

        if (pdpt[pdpt_index] == NULL)
        {
            uint64_t *entry  = setup_entry();
            pdpt[pdpt_index] = entry;
        }

        uint64_t **pd = (uint64_t **)(((uintptr_t)pdpt[pdpt_index] & ~0xfffUL) +
                                      START_KERNEL_MAP);

        // TODO
        // now process use 2M memory(only use pd[0])
        // have to beyond memory space
        if (pd[pd_index] == NULL)
        {
            uint64_t *entry = setup_entry();
            pd[pd_index]    = entry;
        }

        uint64_t **pt = (uint64_t **)(((uintptr_t)pd[pd_index] & ~0xfffUL) +
                                      START_KERNEL_MAP);

        for (int j = 0; j < pt_entry_num; ++j)
        {
            if (pt[pt_index + j] == NULL)
            {
                uint64_t *entry  = setup_entry();
                pt[pt_index + j] = entry;
                // TODO
                // copy elf segment

                int pt_size = 0x1000;

                if (size > 0x1000)
                {
                    size -= 0x1000;
                }
                else
                {
                    pt_size = size;
                }

                uintptr_t load_dest_addr = ((uintptr_t)entry & (~0xfffUL)) +
                                           load_v_addr -
                                           round_down(load_v_addr, 0x1000) + START_KERNEL_MAP;

                memcpy((void *)load_dest_addr,
                       (void *)((uintptr_t)header + offset), pt_size);
            }
        }
    }

    task->context.ret_rip    = (uintptr_t)header->e_entry;
    task->context.ret_rflags = 0x0UL | RFLAGS_IF;

    // TODO this is tempolary stack address
    // You should change to 0x0000 7fff ffff f000
    // and setup page tables
    //    task->rsp = 0x1000;
    task->context.ret_rsp = 0x1000;

    // setup for stack
    uint64_t *pdpt =
        (uint64_t *)((uintptr_t)(task->pml4[0] & (0xfffffffffffff000)) +
                     START_KERNEL_MAP);
    uint64_t *pd, *pt;

    if (pdpt[0] == 0)
    {
        pd = (uint64_t *)early_malloc(1);
        if (pd == 0)
        {
            return false;
        }
        memset(pd, 0, 0x1000);
        pdpt[0] =
            create_entry((uintptr_t)pd, PAGE_READ_WRITE | PAGE_USER_SUPER);
    }
    else
    {
        pd = (uint64_t *)((uintptr_t)(pdpt[0] & 0xfffffffffffff000) +
                          START_KERNEL_MAP);
    }

    if (pd[0] == 0)
    {
        pt = (uint64_t *)early_malloc(1);
        if (pt == 0)
        {
            return false;
        }
        memset(pt, 0, 0x1000);
        pd[0] = create_entry((uintptr_t)pt, PAGE_READ_WRITE | PAGE_USER_SUPER);
    }
    else
    {
        pt = (uint64_t *)(uintptr_t)((pd[0] & 0xfffffffffffff000) +
                                     START_KERNEL_MAP);
    }

    uint64_t *stack_addr = (uint64_t *)early_malloc(1);
    if (stack_addr == 0)
    {
        return false;
    }
    memset(stack_addr, 0, 0x1000);
    pt[0] =
        create_entry((uintptr_t)stack_addr, PAGE_READ_WRITE | PAGE_USER_SUPER);

    return true;
}
