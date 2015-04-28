#ifndef _INCLUDED_MAIN_H_
#define _INCLUDED_MAIN_H_

// #include <stdint.h>
#include <stddef.h>

#include "multiboot.h"

void kernel_entry(uint32_t magic, MULTIBOOT_INFO *multiboot_info);

#endif
