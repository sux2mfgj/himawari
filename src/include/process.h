#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <process.h>
#include <kernel.h>
#include <trap.h>
#include <task_call.h>

struct task_struct
{

    // divide thread struct ?
    // (for reduce architecture depending )

    char name[MODULE_NAME_SIZE];
    //     uintptr_t stack_pointer;
    uintmax_t stack_size;
    // uintptr_t stack_pointer0;

    // TODO delete below struct
    //     struct stack_frame {
    //         uint64_t rip;
    //         uint64_t cs;
    //         uint64_t rflags;
    //         uint64_t rsp;
    //         uint64_t ss;
    //     } *stack_frame;

    uintptr_t rip;
    uint64_t cs;
    uint64_t rflags;
    uintptr_t rsp;
    uint64_t ss;

    uintptr_t entry_addr;

    uint64_t *pml4;

    struct trap_frame_struct context;

    struct message* message_buffer;

    // pid, etc...
};

extern struct task_struct startup_processes[];

//struct task_struct *start_task_array[2];
//int current_task_num;

extern void start_task(struct task_struct *tsk);
extern bool create_kernel_thread(uintptr_t func_addr, uintptr_t stack_end_addr,
                                 uintmax_t stack_size,
                                 struct task_struct *task);
bool create_first_thread(uintptr_t func_addr, uintptr_t stack_end_addr,
                         int stack_size, struct task_struct *task);

extern void schedule(struct trap_frame_struct *trap_frame);
extern void context_switch(struct task_struct *prev, struct task_struct *next,
                           struct trap_frame_struct *trap_frame);

extern bool create_user_process(uintptr_t func_addr, uintptr_t stack_end_addr,
                                uintmax_t stack_size, struct task_struct *task);

extern void irq_eoi(void);
