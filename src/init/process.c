#include <process.h>
#include <segment.h>
#include <x86_64.h>

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
