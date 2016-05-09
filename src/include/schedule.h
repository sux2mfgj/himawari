#pragma once

#include <trap.h>
#include <util.h>

extern bool active_task(struct task_struct *tsk, bool is_head);
extern void suspend_task(struct trap_frame_struct *t_frame, struct Message* msg);

extern struct linked_list *active_head;
extern struct linked_list *suspend_head;

#define current_task container_of(active_head, struct task_struct, active_list)
#define next_task container_of(active_head->next, struct task_struct, active_list)


