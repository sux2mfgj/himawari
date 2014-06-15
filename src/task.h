/*
 * =====================================================================================
 *
 *       Filename:  tss.h
 *
 *    Description:  Task State
 *
 *        Version:  1.0
 *        Created:  05/19/2014 12:00:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef _INCLUDED_TSS_H_
#define _INCLUDED_TSS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


// struct TSS32
// {
//     unsigned int    backlink;
//     unsigned int    esp0;       // Stack pointer. ( Privilage level 0 )
//     unsigned int    ss0;        // Stack segment. ( Privilage level 0 )
//     unsigned int    esp1;       // Stack pointer. ( Privilage level 1 )
//     unsigned int    ss1;        // Stack segment. ( Privilage level 1 )
//     unsigned int    esp2;       // Stack pointer. ( Privilage level 2 )
//     unsigned int    ss2;        // Stack segment. ( Privilage level 2 )
//     unsigned int    cr3;        // Address table of task.
//     unsigned int    eip;        // Instruction pointer.
//     unsigned int    eflags;     // E-flags.
//     unsigned int    eax;        // EAX register.
//     unsigned int    ecx;        // ECX register.
//     unsigned int    edx;        // EDX register.
//     unsigned int    ebx;        // EBX register.
//     unsigned int    esp;        // Stack pointer.
//     unsigned int    ebp;
//     unsigned int    esi;
//     unsigned int    edi;
//     unsigned int    es;
//     unsigned int    cs;
//     unsigned int    ss;
//     unsigned int    ds;
//     unsigned int    fs;
//     unsigned int    gs;
//     unsigned int    ldtr;
//     unsigned int    iomap;

// };


// void set_tss( TSS32* tss, unsigned int cs, unsigned int ds, void (*f)(),
//         unsigned int eflags, unsigned char* esp, unsigned int ss,
//         unsigned char* esp0, unsigned int ss0 );


// task
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
