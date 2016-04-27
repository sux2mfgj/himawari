#pragma once

struct trap_frame_struct {

    //push at trap_entry
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    //TODO
    // should I save fs, gs, es, ds?
    // ...maybe...

    //push at interrupt hander
    uint64_t trap_number;
    uint64_t error_code;

    // push by hardware
    uint64_t ret_rip;
    uint16_t ret_cs;
    uint16_t padding1;
    uint32_t padding2;
    uint64_t ret_rflags;
    uint64_t ret_rsp;
    uint16_t ret_ss;
    uint16_t padding3;
    uint32_t padding4;
};
