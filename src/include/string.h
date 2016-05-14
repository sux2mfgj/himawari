#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

extern void *memset(void *s, int c, size_t count);
extern void *memcpy(void *buf, const void *buf2, size_t n);
extern bool itoa(uint64_t num, char *buf, const uint64_t decimal);
extern int strcmp(const char* s1, const char *s2);

