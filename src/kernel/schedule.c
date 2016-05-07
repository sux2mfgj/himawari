#include <process.h>
#include <kernel.h>
#include <string.h>
#include <x86_64.h>
#include <init.h>
#include <util.h>
#include <segment.h>
#include <schedule.h>

#include <stddef.h>

struct task_struct *tl_active_head;
struct task_struct *tl_suspend_head;
struct task_struct *tl_sending_head;

struct task_struct idle;

// TODO have to use memory manager(library? as kmem_cache?)
uintptr_t mem_pool;

extern void system_task(void);
extern void idle_task(void);

bool init_scheduler(void)
{
    // TODO setup by null
    tl_active_head  = NULL;
    tl_suspend_head = NULL;
    tl_sending_head = NULL;

    // initialize pl_active of system_task

    mem_pool = (uintptr_t)early_malloc(1);
    if (mem_pool == 0)
    {
        return false;
    }

    struct task_struct *t = (struct task_struct *)mem_pool;

    mem_pool += sizeof(struct task_struct);
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

    tl_active_head              = t;
    tl_active_head->active_next = tl_active_head;

    // setup idle task
    idle.pml4 = NULL;
    idle.context.ret_cs = KERNEL_CODE_SEGMENT;
    idle.context.ret_ss = KERNEL_DATA_SEGMENT;
    idle.context.ret_rip = (uintptr_t)&idle_task;
    idle.context.ret_rflags = 0x0UL | RFLAGS_IF;

    stack = early_malloc(1) ;
    if(stack == 0)
    {
        return false;
    }
    idle.context.ret_rsp = stack + 0x1000;
    
    return true;
}

// XXX you should call after initialize
void add_task(struct task_struct *t)
{
    if (tl_active_head == NULL)
    {
        tl_active_head              = t;
        tl_active_head->active_next = tl_active_head;
        return;
    }

    struct task_struct *s_current = tl_active_head;
    while (s_current->active_next != tl_active_head)
    {
        s_current = s_current->active_next;
    }

    s_current->active_next = t;
    t->active_next         = tl_active_head;
}

void start_task(void)
{
    struct task_struct *tsk = tl_active_head;

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
    struct task_struct *current = tl_active_head;
    struct task_struct *next    = tl_active_head->active_next;

    if (current == next)
    {
        return;
    }

    tl_active_head                = next;
    struct task_struct *active_tail = tl_active_head;
    while (active_tail->active_next != tl_active_head)
    {
        active_tail = active_tail->active_next;
    }

    if(current == &idle)
    {
        active_tail->active_next = tl_active_head;
    }
    else 
    {
        active_tail->active_next = current;
        current->active_next   = tl_active_head;
    }

    // TODO
    // think about where I save context
    context_switch(current, next, trap_frame);
}

void sleep_current_task(struct trap_frame_struct *t_frame)
{
    // dequeue
    struct task_struct *current   = tl_active_head;
    struct task_struct *tl_a_tail = tl_active_head;
    struct task_struct* next;

    // list have a task
    if (current->active_next == current)
    {
        tl_active_head = NULL;
        tl_active_head = &idle;
        tl_active_head->active_next = tl_active_head;
        goto enqueue_suspend;
    }

    // some tasks
    while (tl_a_tail->active_next != current)
    {
        tl_a_tail = tl_a_tail->active_next;
    }

    tl_active_head         = current->active_next;
    tl_a_tail->active_next = tl_active_head;

enqueue_suspend:
    // enqueue
    // list have not task 
    if (tl_suspend_head == NULL)
    {
        tl_suspend_head               = current;
        tl_suspend_head->suspend_next = tl_suspend_head;

        goto c_switch;
    }

    // some tasks
    struct task_struct *tl_s_tail = tl_suspend_head;
    while (tl_s_tail->suspend_next != tl_suspend_head)
    {
        tl_s_tail = tl_s_tail->suspend_next;
    }

    tl_s_tail->suspend_next = current;
    current->suspend_next   = tl_suspend_head;

c_switch:

    context_switch(current, tl_active_head, t_frame);
}

bool active_task(struct task_struct *tsk)
{
    // dequeue sleep
    if (tsk->suspend_next == tsk)
    {
        tl_suspend_head   = NULL;
        tsk->suspend_next = NULL;
    }
    else
    {
        struct task_struct *tl_s = tl_suspend_head;
        while (tl_s->suspend_next != tsk)
        {
            if (tl_suspend_head == tl_s->suspend_next)
            {
                return false;
            }
            tl_s = tl_s->suspend_next;
        }
        tl_s->suspend_next = tsk->suspend_next;
    }

    // enqueue active
    if (tl_active_head == NULL)
    {
        tl_active_head              = tsk;
        tl_active_head->active_next = tl_active_head;
        return true;
    }
    struct task_struct *active_tail = tl_active_head;
    while (active_tail->active_next != tl_active_head)
    {
        active_tail = active_tail->active_next;
    }

    active_tail->active_next = tsk;
    tsk->active_next         = tl_active_head;

    return true;
}

void task_sending(struct trap_frame_struct *t_frame)
{
    struct task_struct *sending_tail     = tl_sending_head;
    struct task_struct *current = tl_active_head;
    if (tl_sending_head == NULL)
    {
        tl_sending_head               = current;
        tl_sending_head->sending_next = tl_sending_head;
    }
    else
    {
        while (sending_tail->sending_next != tl_sending_head)
        {
            sending_tail = sending_tail->sending_next;
        }
        sending_tail->sending_next= current;
        current->sending_next = tl_sending_head;
    }

    sleep_current_task(t_frame);
}
