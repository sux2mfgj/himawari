#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

extern void * memset(void * s,int c, size_t count);
extern bool itoa(
        uint64_t num,
        char* buf,
        const uint64_t decimal);

