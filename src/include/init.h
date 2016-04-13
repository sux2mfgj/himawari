#pragma once

#include <stdint.h>
#include <stdbool.h>

// early memory early_memory.c
// should be 64 * n
#define EARLY_MEMORY_PAGE_NUM   64

bool init_early_memory_allocator(uintptr_t kernel_end_addr, uintptr_t available_end, uintptr_t* kernel_end_include_heap);

extern uintptr_t early_malloc(uintmax_t page_num);

// paging (pagetable.c)
extern bool init_pagetable(uintptr_t rounded_kernel_memory_end);

// interrupt (trap.c)
extern bool init_trap(void);

// vram text (vram_text.c)
extern void puts(const char const* text);
extern bool itoa(
        uint64_t num,
        char* buf,
        const uint64_t decimal);


