#pragma once

#include <init.h>

// >> 6 is divide sizeof(uint64_t)
#define bitmap_size ((EARLY_MEMORY_PAGE_NUM + (sizeof(uint64_t) * 8) + 1) >> 6)

extern uint64_t bitmap[bitmap_size];

