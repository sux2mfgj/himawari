#pragma once

#include <init.h>

enum {
    MEMORY,
    BOOT_MODULES_NUM,
};

// void nothing(void* ptr)
// {
// }

// kernel/elf.c
extern void load_elf_image(void);

