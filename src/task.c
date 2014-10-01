
#include "task.h"
#include "func.h"
#include "graphic.h"


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


