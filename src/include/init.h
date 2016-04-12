#pragma once

#include <stdint.h>
#include <stdbool.h>

// early memory
// should be 64 * n
#define EARLY_MEMORY_PAGE_NUM   64

extern bool init_early_memory_allocator(uintptr_t kernel_end_addr, uintptr_t available_end);
extern uintptr_t early_malloc(uintmax_t page_num);
