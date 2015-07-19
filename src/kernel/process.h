#ifndef _INCLUDED_PROCESS_H_
#define _INCLUDED_PROCESS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum  {
    RUNNING,
    READY,
    BLOCKING,
} process_state;

typedef struct{
    uintptr_t esp;
    uintptr_t eip;
} context;

typedef struct _task_struct{
    struct _task_struct* next_ptr;
    uintptr_t page_directory_table;
    context context;
    char name[0x10];
} task_struct;

bool init_process(void);
void task_switch(task_struct *prev, task_struct *next);
void __switch_to(void);


// for sceduler 
// typedef struct _node{
//     struct _node* next_ptr; 
//     task_struct* task_struct;
// } task_node;

void schedule(void);
void append_take_node(task_struct* head, task_struct* node);

void start_other_process(void);

#endif
