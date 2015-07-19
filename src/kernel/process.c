#include "process.h"

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
/*     task_switch(); */
}
