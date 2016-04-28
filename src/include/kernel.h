#pragma once

#include <init.h>

enum task {
    MEMORY,
    SCHEDULE,
    BOOT_MODULES_NUM,
};

// void nothing(void* ptr)
// {
// }

// kernel/elf.c
extern void load_elf_image(void);

const static int MODULE_NAME_SIZE = 32;
