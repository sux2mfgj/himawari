
#include "task.h"
#include "func.h"
#include "graphic.h"
#include "k_memory.h"

bool init_task()
{

    current_task = (task_struct *)memory_allocate(sizeof(task_struct));
    current_task->page_directory_table = NULL;
    current_task->state = RUNNIG;
    current_task->kind = THREAD;
    current_task->next_task = current_task;
    current_task->id = 0;

    task_list_lock = false;

    return true;
}

void task_switch(task_struct *prev, task_struct *next)
{
    asm volatile(
        "pushfl;"
        "pushl %%ebp;"
        "movl %%esp, %[prev_esp];"
        "movl %[next_esp], %%esp;"
        "movl $1f, %[prev_eip];"
        "pushl %[next_eip];"
        "jmp __switch_to;"
        "1:;"
        "popl %%ebp;"
        "popfl;"
        // output parameters
        : [prev_esp] "=m"(prev->context.esp), [prev_eip] "=m"(prev->context.eip)
          // input parameters
        : [next_esp] "m"(next->context.esp), [next_eip] "m"(next->context.eip),
          "a"(prev), "d"(next));
}
/*
  switch_to: #void task_switch(TASK_MANAGEMENT_DATA *prev, TASK_MANAGEMENT_DATA
*next)

#     pusha
    pushf
    pushl %ebp

#store
    movl 12(%esp), %ebp
    movl %esp, 0(%ebp) # prev->esp = %esp
#     movl restore, %eax
    movl $1f, 4(%ebp) #prev->eip = $restore

#load
    movl 16(%esp), %ebp
    movl 0(%ebp), %esp     # %esp = next->esp
#    pushl $1f
    pushl 4(%ebp)       #push next->eip

#   test
#     pushl %eax
#     pushl %eax
    jmp switch_to
1:
#     jmp switch_to
    popl %ebp
    popf
#     popa
    ret

 */

void __switch_to()
{
    io_sti();
    return;
}

static void lock_task_list()
{
    while (task_list_lock)
        ;
    task_list_lock = true;
    return;
}

static void unlock_task_list()
{
    task_list_lock = false;
    return;
}

bool create_kernel_thread(void *entry)
{
    task_struct *thread = (task_struct *)memory_allocate(sizeof(task_struct));
    if (NULL == thread) {
        return false;
    }

    thread->page_directory_table = NULL;
    thread->state = RUNNIG;
    thread->kind = THREAD;
    thread->id = ++pid;
    thread->context.eip = entry;

    uint32_t *bp = (uint32_t *)memory_allocate_4k(1);
    thread->context.esp = (uint32_t *)((uintptr_t)bp + 0x1000);
/*     bp[0x1000] = (uintptr_t)bp; */
/*     bp[0x0fff] = 0x202; */
    thread->context.esp0 = NULL;

    lock_task_list();
    task_struct *tmp = current_task->next_task;
    current_task->next_task = thread;
    thread->next_task = tmp;
    unlock_task_list();

    return true;
}

void scheduler_tick()
{
        scheduler();
}

/* static bool scheduler(void) */
bool scheduler(void)
{
/*     lock_task_list(); */
    task_struct *prev = current_task;
    current_task = current_task->next_task;
/*     unlock_task_list(); */
    task_switch(prev, current_task);

    return true;
}

void print_pid_test()
{
    task_struct *current = current_task->next_task;
    printf(TEXT_MODE_SCREEN_RIGHT, "%d->", current_task->id);
    while (current != current_task) {
        printf(TEXT_MODE_SCREEN_RIGHT, "%d->", current->id);
        current = current->next_task;
    }
}
