#pragma once

#include <pvh.h>

#include <stddef.h>

int vm_init(struct hvm_memmap_table_entry *entries, size_t nentry);
int vm_map_device_straight(uint64_t base, size_t npages);
int vm_map_ram_straight(uint64_t base, size_t npages);
