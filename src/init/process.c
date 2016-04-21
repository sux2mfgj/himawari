#include <process.h>
#include <segment.h>
#include <x86_64.h>

//TODO
bool init_scheduler(struct task_struct* first, struct task_struct* second)
{
    start_task_array[0]  = first;
    start_task_array[1]  = second;
    current_task_num = 1;
    return true;
}

void start_kernel_thread(void)
{
    __asm__ volatile(
            "pushq %0;\n\t" 
            "pushq %1;\n\t" 
            "pushq %2;\n\t" 
            "pushq %3;\n\t" 
            "pushq %4;\n\t" 
            "iretq;"
            :"=m"(start_task_array[0]->stack_frame->ss),
            "=m"(start_task_array[0]->stack_frame->rsp),
            "=m"(start_task_array[0]->stack_frame->rflags),
            "=m"(start_task_array[0]->stack_frame->cs),
            "=m"(start_task_array[0]->stack_frame->rip));

/*         "movq %0, %%rsp;\n\t" */
/*         "iretq;" */
/*         : "=m"(start_task_array[0]->stack_pointer)); */
}

void schedule(void)
{
    int prev, next = 0;

    prev = current_task_num;
    if(current_task_num == 0)
    {
        next = 1;
    }
    else if(current_task_num == 1) {
        next = 0;
    }
    current_task_num = next;

    __asm__ volatile(
        "movq %%rsp, %0\n\t"
        "movq %1, %%rsp"
        :"=m"(start_task_array[prev]->stack_pointer)
        :"m"(start_task_array[next]->stack_pointer));
                

}

bool create_kernel_thread(
        uintptr_t func_addr,
        uintptr_t stack_end_addr,
        uintmax_t stack_size,
        struct task_struct* task)
{
    uintptr_t stack_head = (stack_end_addr 
        + stack_size - sizeof(struct stack_frame));

    stack_head = (stack_head >> 8) << 8;
    task->stack_pointer = stack_head;
    task->stack_size = stack_size;

    task->stack_frame = (struct stack_frame*)(stack_head);
        
    task->stack_frame->rip  = func_addr;
    task->stack_frame->cs = KERNEL_CODE_SEGMENT;
    task->stack_frame->rflags = 0x0UL | RFLAGS_IF;
    task->stack_frame->rsp = stack_head;
    task->stack_frame->ss = KERNEL_DATA_SEGMENT;

    return true;
}
