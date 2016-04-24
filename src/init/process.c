#include <init.h>
#include <kernel.h>
#include <process.h>
#include <segment.h>
#include <x86_64.h>
#include <string.h>
#include <util.h>
#include <elf.h>

// TODO
bool init_scheduler(struct task_struct *first, struct task_struct *second)
{
    start_task_array[0] = first;
    start_task_array[1] = second;
    current_task_num    = 0;
    return true;
}

void start_first_task() { start_task(start_task_array[0]); }

void start_task(struct task_struct *tsk)
{
    __asm__ volatile("pushq %0;\n\t"
                     "pushq %1;\n\t"
                     "pushq %2;\n\t"
                     "pushq %3;\n\t"
                     "pushq %4;\n\t"
                     "iretq;"
                     : "=m"(tsk->ss), "=m"(tsk->rsp), "=m"(tsk->rflags),
                       "=m"(tsk->cs), "=m"(tsk->rip));

    /*         "movq %0, %%rsp;\n\t" */
    /*         "iretq;" */
    /*         : "=m"(start_task_array[0]->stack_pointer)); */
}

void _switch_to(void) { return; }

void schedule(void)
{
    int prev, next = 0;

    prev = current_task_num;
    if (current_task_num == 0)
    {
        next = 1;
    }
    else if (current_task_num == 1)
    {
        next = 0;
    }
    current_task_num = next;

    /*     __asm__ volatile( */
    /*         "movq %%rsp, %0\n\t" */
    /*         "movq %1, %%rsp" */
    /*         :"=m"(start_task_array[prev]->stack_pointer) */
    /*         :"m"(start_task_array[next]->stack_pointer)); */

    __asm__ volatile(
        "movq %%rsp, %0;\n\t"
        "movq $1f, %1;\n\t"
        "movq %2, %%rsp;\n\t"
        "pushq %3;\n\t"
        "jmp _switch_to;\n\t"
        "1:;"
        : "=m"(start_task_array[prev]->rsp), "=m"(start_task_array[prev]->rip)
        : "m"(start_task_array[next]->rsp), "m"(start_task_array[next]->rip));
}

bool create_first_thread(uintptr_t func_addr, uintptr_t stack_end_addr,
                         int stack_size, struct task_struct *task)
{
    uintptr_t stack_head = stack_end_addr + stack_size;
    task->rip            = func_addr;
    task->cs             = KERNEL_CODE_SEGMENT;
    task->rflags         = 0x0UL | RFLAGS_IF;
    task->rsp            = stack_head;
    task->ss             = KERNEL_DATA_SEGMENT;

    return true;
}

bool create_kernel_thread(uintptr_t func_addr, uintptr_t stack_end_addr,
                          uintmax_t stack_size, struct task_struct *task)
{
    uintptr_t stack_head = stack_end_addr + stack_size;

    stack_head = (stack_head >> 8) << 8;
    /*     task->stack_pointer = stack_head; */
    task->stack_size = stack_size;

    // task->stack_frame = (struct stack_frame*)(stack_head);

    /*     task->rip  = func_addr; */
    //    task->rip = (uintptr_t)start_kernel_thread;
    task->cs     = KERNEL_CODE_SEGMENT;
    task->rflags = 0x0UL | RFLAGS_IF;
    task->rsp    = stack_head;
    task->ss     = KERNEL_DATA_SEGMENT;

    // for start_kernel_thread argument
    *(uintptr_t *)stack_head = (uintptr_t)task;

    return true;
}

bool create_user_process(uintptr_t func_addr, uintptr_t stack_end_addr,
                         uintmax_t stack_size, struct task_struct *task)
{
    uintptr_t stack_head = stack_end_addr + stack_size;

    task->stack_size = stack_size;

    task->rip    = (uintptr_t)func_addr;
    task->cs     = USER_CODE_SEGMENT;
    task->rflags = 0x0UL | RFLAGS_IF;
    task->rsp    = stack_head;
    task->ss     = USER_DATA_SEGMENT;

    return true;
}

struct task_struct startup_processes[BOOT_MODULES_NUM];
bool setup_server_process(uintptr_t elf_header, struct task_struct *task)
{

    task->pml4 = create_pml4();
    if (task->pml4 == 0)
    {
        return false;
    }
    task->cs = SERVER_CODE_SEGMENT;
    task->ss = SERVER_DATA_SEGMENT;

    struct elf_header *header = (struct elf_header *)elf_header;

    struct elf_program_header *p_header =
        (struct elf_program_header *)(elf_header + header->e_phoff);

    for (int i = 0; i < header->e_phnum; ++i)
    {
        uint64_t offset       = p_header[i].p_offset;
        uintptr_t load_v_addr = p_header[i].p_vaddr;
        int size              = p_header[i].p_memsz;
        if(size == 0) {
            continue;
        }

        {
            char buf[32];
            itoa(load_v_addr, buf, 16);
            puts("0x");
            puts(buf);
            puts("(");
            itoa(size, buf, 16);
            puts(buf);
            puts(")");
            puts("\n");
        }

        // server process can use memory(1G)
        // TODO beyond usbale memory range
        if ((load_v_addr + size) >= 0x40000000)
        {
            return false;
        }

        uint64_t *pdpt = (uint64_t *)early_malloc(1);
        if (pdpt == 0)
        {
            return false;
        }
        memset(pdpt, 0, 0x1000);

        uint64_t *pd = (uint64_t *)early_malloc(1);
        if (pd == 0)
        {
            return false;
        }
        memset(pd, 0, 0x1000);

        task->pml4[0] =
            create_entry((uintptr_t)pdpt, PAGE_READ_WRITE | PAGE_USER_SUPER);

        pdpt[0] =
            create_entry((uintptr_t)pd, PAGE_READ_WRITE | PAGE_USER_SUPER);

        int pd_index            = load_v_addr / 0x200000;
        //int pd_num              = (load_v_addr + size) / 0x200000;
        int pd_num              = (size / 0x200000) + 1;
        uintptr_t v_addr_flower = round_down(load_v_addr, 0x1000);
        for (int j = 0; j < pd_num; ++j)
        {
            uint64_t *pt = (uint64_t *)early_malloc(1);
            if (pt == 0)
            {
                return false;
            }
            memset(pt, 0, 0x1000);

            for (int k = 0; k < 512; ++k)
            {

                pt[i] = create_entry(early_malloc(1),
                                     PAGE_READ_WRITE | PAGE_USER_SUPER);

                // TODO below branch should replace above ?
                if ((v_addr_flower + ((uintptr_t)k << 12) + (j * 0x200000)) >
                    (load_v_addr + size))
                {
                    break;
                }
            }
            pd[j + pd_index] =
                create_entry((uintptr_t)pt, PAGE_READ_WRITE | PAGE_USER_SUPER);
        }
    }

    task->rip    = header->e_entry;
    task->rflags = 0x0UL | RFLAGS_IF;
    // TODO this is tempolary stack address
    // You should change to 0x0000 7fff ffff f000
    // and setup page tables
    task->rsp = 0x1000;
    return true;
}
