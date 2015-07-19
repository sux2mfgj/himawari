#ifndef _INCLUDED_PROCESS_H_
#define _INCLUDED_PROCESS_H_

#include <stdint.h>

typedef enum  {
    RUNNING,
    READY,
    BLOCKING,
} process_state;

typedef struct{
    uintptr_t esp;
    uintptr_t eip;
} context;

typedef struct{
    uintptr_t page_directory_table;
    context context;
} task_struct;

void task_switch(task_struct *prev, task_struct *next);
void __switch_to(void);

void schedule(void);

#endif
