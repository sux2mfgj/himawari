#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <page.h>
#include <task_call.h>


// early memory early_memory.c
// should be 64 * n
#define EARLY_MEMORY_PAGE_NUM   64

// bool init_early_memory_allocator(uintptr_t kernel_end_addr, uintptr_t available_end, uintptr_t* kernel_end_include_heap);
extern bool init_early_memory_allocator(struct memory_info* m_info) ;

extern uintptr_t early_malloc(uintmax_t page_num);

// paging (pagetable.c)
extern bool init_pagetable(uintptr_t rounded_kernel_memory_end);
extern uint64_t* create_pml4(void);

// interrupt (trap.c)
extern bool init_trap(void);
extern void set_gate_descriptor(int descriptor_num,
                         unsigned type,
                         uintptr_t offset,
                         unsigned ist,
                         unsigned dpl);

// vram text (vram_text.c)
extern void puts(const char * text);
extern void enable_virtual_memory(void);

// local apic (local_apic.c)
extern bool init_local_apic(void);

// pic & pit (pic.c)
extern bool init_pic(void);

#include <process.h>
// process (process.c)
struct task_struct;
extern void start_first_task(void);
extern bool setup_server_process(uintptr_t elf_header, struct task_struct *task, char* name);

// schduler (schedule.c)
extern bool init_scheduler(void);
extern void start_task(void);

// system call (syscall.c)

extern bool init_syscall(void);

#define BOOT_MODULES_NUM ServerNum

