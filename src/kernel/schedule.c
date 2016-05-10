#include <process.h>
#include <kernel.h>
#include <string.h>
#include <x86_64.h>
#include <init.h>
#include <util.h>
#include <segment.h>
#include <schedule.h>

#include <stddef.h>

struct linked_list *active_head;
struct linked_list *suspend_head;


struct task_struct idle;

// TODO have to use memory manager(library? as kmem_cache?)
uintptr_t mem_pool;

extern void system_task(void);
extern void idle_task(void);

static char system_name[MODULE_NAME_SIZE] = "system";

bool init_scheduler(void)
{
    // TODO setup by null

    active_head = NULL;
    suspend_head = NULL;

    // initialize active_head (containing a task about system_task)

    mem_pool = (uintptr_t)early_malloc(1);
    if (mem_pool == 0)
    {
        return false;
    }

    struct task_struct *t = (struct task_struct *)mem_pool;

    mem_pool += sizeof(struct task_struct);
    mem_pool = round_up(mem_pool, 0x100);

    memcpy(t->name, system_name, MODULE_NAME_SIZE);

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

    list_init(&t->active_list);
    active_head = &t->active_list;

    // setup idle task
    idle.pml4 = NULL;
    idle.context.ret_cs = KERNEL_CODE_SEGMENT;
    idle.context.ret_ss = KERNEL_DATA_SEGMENT;
    idle.context.ret_rip = (uintptr_t)&idle_task;
    idle.context.ret_rflags = 0x0UL | RFLAGS_IF;
    memcpy(&idle.name, "idle", sizeof(char) * MODULE_NAME_SIZE);

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
    if (active_head == NULL)
    {
        list_init(&t->active_list);
        active_head = &t->active_list;
        return;
    }

    enqueue(active_head, &t->active_list);
}

void start_task(void)
{
    struct task_struct *tsk = current_task;

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
    struct task_struct *current = current_task;
    struct task_struct *next    = next_task;

    // don't need call context switch
    if (current == next)
    {
        return;
    }

    // update active_head
    active_head = active_head->next; 

    //TODO 
    // must consider idle task

    //TODO
    // think about where I save context
    context_switch(current, next, trap_frame);
}

void sleep_current_task(struct trap_frame_struct *t_frame)
{
    // dequeue
    struct task_struct *prev = current_task;

    struct linked_list* next = dequeue(active_head);

    if(next == NULL)
    {
        list_init(&idle.active_list);
        active_head = &idle.active_list;
        goto enqueue_suspend;
    }

    // when list have more than 2 tasks
    active_head = next;

enqueue_suspend:
    // enqueue

    // list have not task 
    if (suspend_head == NULL)
    {
        list_init(&current_task->suspend_list);
        suspend_head = &current_task->suspend_list;

        goto c_switch;
    }

    // some tasks
    enqueue(suspend_head, &current_task->suspend_list);

c_switch:

    context_switch(prev, current_task, t_frame);
}

bool active_task(struct task_struct *tsk, bool is_head)
{
    // dequeue from sleep
    suspend_head = dequeue(&tsk->suspend_list);

    // enqueue active
    if (active_head == NULL)
    {
        active_head = list_init(&tsk->active_list);
        return true;
    }

    enqueue(active_head, &tsk->active_list);
    if(is_head)
    {
        active_head = &tsk->active_list;
    }

    return true;
}

void suspend_task(struct trap_frame_struct *t_frame, struct Message* msg)
{
    memcpy(&current_task->msg_buf, msg, sizeof(struct Message));

    if(msg->type == Receive)
    {
        current_task->msg_addr = msg;
    }

    if (suspend_head == NULL)
    {
        suspend_head = list_init(&current_task->suspend_list);
    }
    else
    {
        enqueue(suspend_head, &current_task->suspend_list);
    }

    sleep_current_task(t_frame);
}
