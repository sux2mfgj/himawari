#pragma once

#include <stdbool.h>
#include <stdint.h>


struct task_struct {

    // divide thread struct ?
    // (for reduce architecture depending )

    uintptr_t stack_pointer;
    uintmax_t stack_size;
    //uintptr_t stack_pointer0;
    
    struct stack_frame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    } *stack_frame;

    //pid
};

struct task_struct* start_task_array[2];
int current_task_num;

extern bool create_kernel_thread(
        uintptr_t func_addr,
        uintptr_t stack_end_addr,
        uintmax_t stack_size,
        struct task_struct* task);
