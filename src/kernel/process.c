#include "process.h"
#include "kmemory.h"
#include "system.h"

static task_node* task_node_head;
static task_node* current_task;

bool init_process(void)
{
    task_node_head = kmalloc(sizeof(task_node));
    task_node_head->next_ptr = task_node_head;

    task_node_head->task_struct.page_directory_table = NULL;
    task_node_head->task_struct.context.eip = system;
    task_node_head->task_struct.context.esp = NULL;

    current_task = task_node_head;

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
            //output parameters
            :[prev_esp] "=m" (prev->context.esp), [prev_eip] "=m" (prev->context.eip)
            //input parameters
            :[next_esp] "m" (next->context.esp), [next_eip] "m" (next->context.eip),
            "a" (prev), "d" (next));
}

void __switch_to(void) {}

void schedule(void)
{
    task_struct* prev = current_task;
    task_struct* next = current_task->next_ptr;

    if (prev != next){
        task_switch(prev, next);
    }
}

void append_take_node(task_node* head, task_node* node)
{
    task_node* tmp = head;
    while (tmp->next_ptr != head) {
        tmp = tmp->next_ptr;
    }     

    node->next_ptr = head;
    tmp->next_ptr = node;
}
