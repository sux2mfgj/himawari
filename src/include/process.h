#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <process.h>
#include <kernel.h>
#include <trap.h>
#include <task_call.h>
#include <list.h>

struct task_struct
{

    // divide thread struct ?
    // (for reduce architecture depending )
    char name[MODULE_NAME_SIZE];
    uintmax_t stack_size;

    uintptr_t rip;
    uint64_t cs;
    uint64_t rflags;
    uintptr_t rsp;
    uint64_t ss;

    uintptr_t entry_addr;

    uint64_t *pml4;

    struct trap_frame_struct context;

    //struct Message msg_buf;
    //struct Message *msg_addr;
    struct MessageInfo msg_info;

    struct linked_list active_list;
    struct linked_list suspend_list;

    //    struct task_struct* active_next;
    //    struct task_struct* suspend_next;
    //    struct task_struct* sending_next;
    //    struct task_struct* receving_next;;

    // pid, etc...
};

extern void schedule(struct trap_frame_struct *trap_frame);
extern void context_switch(struct task_struct *prev, struct task_struct *next,
                           struct trap_frame_struct *trap_frame);

extern void irq_eoi(void);
