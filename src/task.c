
#include "task.h"
#include "func.h"
#include "graphic.h"


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
/*
  switch_to: #void task_switch(TASK_MANAGEMENT_DATA *prev, TASK_MANAGEMENT_DATA *next)

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
    return;
}


