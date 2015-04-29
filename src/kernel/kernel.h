#ifndef _INCLUDED_MAIN_H_
#define _INCLUDED_MAIN_H_

// #include <stdint.h>
#include <stddef.h>

#include "multiboot.h"

void kernel_entry(const uint32_t magic, const MULTIBOOT_INFO *multiboot_info);

#endif
