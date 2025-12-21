#pragma once

#include <stdint.h>

int core_init(uint64_t boot_core_id);
int core_register_app_core_id(uint64_t core_id);
int core_start_aps(void);
