#pragma once

#include <hm/pmm.h>

int vmm_init(void);
int vmm_map_ram_straight(struct mem_block *block);
