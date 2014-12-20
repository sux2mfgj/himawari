#ifndef _INCLUDED_TASK_H__
#define _INCLUDED_TASK_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "lib.h"

#define KERNEL_STACK_SIZE 1024

typedef struct tss_struct {
    uint16_t backlink;
    uint32_t esp0;
    uint16_t ss0;
    uint32_t esp1;
    uint16_t ss1;
    uint32_t esp2;
    uint16_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t f5;
    uint16_t cs;
    uint16_t f6;
    uint16_t ss;
    uint16_t f7;
    uint16_t ds;
    uint16_t f8;
    uint16_t fs;
    uint16_t f9;
    uint16_t gs;
    uint16_t f10;
    uint16_t ldt;
    uint16_t f11;
    uint16_t t;
    uint16_t iobase;
} tss_struct;

static tss_struct tss0;

typedef enum {
    RUNNIG,
    READY,
    BLOCKED
} process_state;

typedef enum {
    PROCESS,
    THREAD
} kind;

typedef struct context {
    uint32_t* esp;
    uint32_t* esp0;
    uint32_t* eip;
} context;

typedef struct task_struct {
    uint32_t* page_directory_table;
    process_state state;
    kind kind;
    uint32_t id;
    context context;
    struct task_struct* next_task;
    //     uint32_t* kernel_stack;
    //     struct task_struct* parent;
    //     char task_name[16];
} task_struct;

static task_struct init_process;

// union thread_union {
//     thread_info thread_info;
//     uint32_t stack[KERNEL_STACK_SIZE];
// };

void task_switch(task_struct* prev, task_struct* next);
void __switch_to(void);

// scheduler
bool create_kernel_thread(void* entry);
bool create_init_thread(void);
static bool __create_kernel_thread(void* entry, bool is_init);
static bool task_list_lock;
static void lock_task_list();
static void unlock_task_list();

bool init_task(void);
static task_struct* current_task;

void scheduler_tick(void);
static bool scheduler(void);

bool exec_init(void);
void init(void);
void task1(void);
void task2(void);

void print_pid_test();



static uint32_t pid = 0;
#endif
