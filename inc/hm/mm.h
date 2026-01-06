#pragma once

#include <pvh.h>
#include <stddef.h>

int mm_early_init(void);
int mm_init(struct hvm_memmap_table_entry *entries, size_t nentries);
void *mm_alloc(size_t size);
void mm_free(void *ptr);

#define PAGE_SIZE 0x1000
