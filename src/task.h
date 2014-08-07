#ifndef _INCLUDED_TSS_H_
#define _INCLUDED_TSS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct TASK_MANAGEMENT_DATA
{
    uintptr_t esp;
    uintptr_t eip;
};

void task_switch_c(uint32_t task1_num, uint32_t task2_num);
void set_task(uint32_t task, void (*f)(), uint8_t* esp);
// struct TASK_MANAGEMENT_DATA* switch_to(struct TASK_MANAGEMENT_DATA *prev,
//         struct TASK_MANAGEMENT_DATA *next);
void switch_to(void);

#endif
