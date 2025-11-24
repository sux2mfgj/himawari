#pragma once

#include <pvh.h>
#include <stddef.h>

struct mem_block {
  uint64_t base;
  int npages;
};

int pmm_init(struct hvm_memmap_table_entry *table, int nentry);
int pmm_get_phys_mem_info(struct mem_block **mem_info, int *nentry);
void *pmm_alloc(size_t size);
