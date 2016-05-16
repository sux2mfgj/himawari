#pragma once

#include <init.h>
#include <page.h>

// >> 6 is divide sizeof(uint64_t)
//#define bitmap_size ((EARLY_MEMORY_PAGE_NUM + (sizeof(uint64_t) * 8) - 1) >> 6)

#define EARLY_MEMORY_SIZE 0x01000000 // 16MB (after ISA memory hole)
#define EARLY_MEMORY_PAGE_NUM (EARLY_MEMORY_SIZE / PAGE_SIZE)
#define BIT_NUMBER_OF_BYTE 8
#define BITMAP_UNIT64_NUM (EARLY_MEMORY_PAGE_NUM / ((sizeof(uint64_t) * BIT_NUMBER_OF_BYTE)))
#define BITMAP_ENTRY_NUM BITMAP_UNIT64_NUM

extern uint64_t bitmap[BITMAP_ENTRY_NUM];

