#include <process.h>
#include <kernel.h>
#include <string.h>
#include <x86_64.h>
#include <init.h>
#include <util.h>
#include <segment.h>

#include <stddef.h>

struct process_list *pl_head;

// TODO have to use memory manager(library? as kmem_cache?)
uintptr_t mem_pool;

extern void system_task(void);

bool init_scheduler(void)
{
    // initialize pl_head of system_task

    mem_pool = (uintptr_t)early_malloc(1);
    if (mem_pool == 0)
    {
        return false;
    }

    struct task_struct *t = (struct task_struct *)mem_pool;

    mem_pool = round_up(mem_pool, 0x100);

    t->pml4           = NULL;
    t->context.ret_cs = KERNEL_CODE_SEGMENT;
    t->context.ret_ss = KERNEL_DATA_SEGMENT;

    t->context.ret_rip = (uintptr_t)&system_task;

    t->context.ret_rflags = 0x0UL | RFLAGS_IF;

    uintptr_t stack = early_malloc(1);
    if (stack == 0)
    {
        return false;
    }

    t->context.ret_rsp = stack + 0x1000;

    pl_head = (struct process_list *)mem_pool;

    mem_pool = round_up(mem_pool, 0x100);

    pl_head->next = pl_head;
    pl_head->task = t;

    return true;
}

void add_task(struct task_struct *t)
{
    struct process_list *l = (struct process_list *)mem_pool;

    mem_pool = round_up(mem_pool, 0x100);

    l->task = t;

    struct process_list *s_current = pl_head;
    while (s_current->next != pl_head)
    {
        s_current = s_current->next;
    }

    s_current->next = l;
    l->next         = pl_head;
}

void start_task(void)
{
    struct task_struct *tsk = pl_head->task;

    if (tsk->pml4 != NULL)
    {
        load_cr3(tsk->pml4);
    }

    __asm__ volatile("pushq %0;\n\t"
                     "pushq %1;\n\t"
                     "pushq %2;\n\t"
                     "pushq %3;\n\t"
                     "pushq %4;\n\t"
                     "iretq;"
                     : "=m"(tsk->context.ret_ss), "=m"(tsk->context.ret_rsp),
                       "=m"(tsk->context.ret_rflags), "=m"(tsk->context.ret_cs),
                       "=m"(tsk->context.ret_rip));

    // not reach here
}

void _switch_to(void) { return; }

void context_switch(struct task_struct *prev, struct task_struct *next,
                    struct trap_frame_struct *trap_frame)
{
    if (next->pml4 != NULL)
    {
        load_cr3(next->pml4);
    }

    memcpy(&prev->context, trap_frame, sizeof(struct trap_frame_struct));
    memcpy(trap_frame, &next->context, sizeof(struct trap_frame_struct));
}

void schedule(struct trap_frame_struct *trap_frame)
{
    struct process_list *current = pl_head;
    struct process_list *next    = pl_head->next;

    if (current == next)
    {
        return;
    }

    pl_head                        = next;
    struct process_list *s_current = pl_head;
    while (s_current->next != pl_head)
    {
        s_current = s_current->next;
    }

    s_current->next = current;
    current->next   = pl_head;

    // TODO
    // think about where I save context
    context_switch(current->task, next->task, trap_frame);
}
