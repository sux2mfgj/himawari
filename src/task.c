/*
 * =====================================================================================
 *
 *       Filename:  tss.c
 *
 *    Description:  j
 *
 *        Version:  1.0
 *        Created:  05/19/2014 12:08:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include "task.h"
#include "func.h"
#include "graphic.h"







/* void set_tss(struct TSS32* tss, */
/*                 unsigned int cs, */
/*                 unsigned int ds, */
/*                 void (*f)(), */
/*                 unsigned int eflags, */
/*                 unsigned char* esp, */
/*                 unsigned int ss, */
/*                 unsigned char* esp0, */
/*                 unsigned int ss0 ) */
/* { */
/*     memset(tss, 0, sizeof(struct TSS32)); */
/*     tss->cs     = cs; */
/*     tss->eip    = (uintptr_t) f; */
/*     //printf_str( 80, 60, "%d", tss->eip ); */
/*     tss->eflags = eflags; */
/*     tss->esp    = (uintptr_t)esp; */
/*     //printf_str( 40, 40, "%d", tss->esp ); */
/*     tss->ds     = ds; */
/*     tss->es     = ds; */
/*     tss->fs     = ds; */
/*     tss->gs     = ds; */
/*     tss->ss     = ss; */
/*     tss->esp0   = (uintptr_t) esp0; */
/*     //printf_str( 100, 100, "%d", tss->esp0 ); */
/*     tss->ss0    = ss0; */
/*     tss->iomap  = 0x40000000; */
/*     //tss->esp0 = tss->esp1 = tss->esp2 = tss->esp; */
/*     //tss->ss0 = tss->ss1 = tss->ss2 = tss->ss; */
/* } */


// task

static struct TASK_MANAGEMENT_DATA task_management_data[3];

void task_switch_c(uint32_t task1_num, uint32_t task2_num)
{
/*     printf(TEXT_MODE_SCREEN_RIGHT, "before %d: 0x%x", */
/*             task1_num, task_management_data[task1_num].eip); */
/*     printf(TEXT_MODE_SCREEN_RIGHT, "before %d: 0x%x", */
/*             task1_num, task_management_data[task2_num].eip); */
    task_switch(&task_management_data[task1_num], &task_management_data[task2_num]);
/*     printf(TEXT_MODE_SCREEN_RIGHT, "after %d: 0x%x", */
/*             task1_num, task_management_data[task1_num].eip); */
/*     printf(TEXT_MODE_SCREEN_RIGHT, "after %d: 0x%x", */
/*             task1_num,  task_management_data[task2_num].eip); */
}

void set_task(uint32_t task_number, void (*f)(), uint8_t* esp)
{
/*     esp--; */
/*     *esp = (uintptr_t)f; */
/*     ++esp; */

    task_management_data[task_number].esp = (uintptr_t)esp;
/*     _set_task(&(task_management_data[task_number].eip)); */
    task_management_data[task_number].eip = (uintptr_t)f;


    printf(TEXT_MODE_SCREEN_RIGHT, "task%d: esp: 0x%x", task_number, (uintptr_t)esp);
    printf(TEXT_MODE_SCREEN_RIGHT, "task%d: eip: 0x%x", task_number, (uintptr_t)f);
}

void switch_to()
{
    printf(TEXT_MODE_SCREEN_RIGHT, "in switch_to");
    return;
}


