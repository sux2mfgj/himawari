#pragma once

#include <stdbool.h>
#include "boot_arg.h"

bool init_memory_manager(size_t number_of_entries,
                         struct memory_info *meminfo_array);

bool malloc_4k(uintptr_t *dest);
