#pragma once

#include <trap.h>

extern void task_sending(struct trap_frame_struct* t_frame, struct Message* msg);
extern void task_receiving(struct trap_frame_struct* t_frame, struct Message* msg);
extern bool active_task(struct task_struct *tsk);

extern struct task_struct *tl_active_head;
extern struct task_struct *tl_sending_head;
extern struct task_struct *tl_receving_head;

