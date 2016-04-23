#pragma once

#include <stdbool.h>
#include <stdint.h>


struct task_struct {

    // divide thread struct ?
    // (for reduce architecture depending )

//     uintptr_t stack_pointer;
    uintmax_t stack_size;
    //uintptr_t stack_pointer0;
    
    //TODO delete below struct
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

    //pid, etc...
};

struct task_struct* start_task_array[2];
int current_task_num;

extern void start_task(struct task_struct* tsk);
extern bool create_kernel_thread(
        uintptr_t func_addr,
        uintptr_t stack_end_addr,
        uintmax_t stack_size,
        struct task_struct* task);
bool create_first_thread(
        uintptr_t func_addr,
        uintptr_t stack_end_addr,
        int stack_size,
        struct task_struct* task
        );

extern void schedule(void);


extern bool create_user_process(
        uintptr_t func_addr,
        uintptr_t stack_end_addr,
        uintmax_t stack_size,
        struct task_struct* task);
