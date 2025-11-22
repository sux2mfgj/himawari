#pragma once

#include <pvh.h>
#include <stddef.h>

int pmm_init(struct hvm_memmap_table_entry *table, int nentry);
void *pmm_alloc(size_t size);
