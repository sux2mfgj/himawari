#pragma once

#include <stddef.h>

#define container_of(ptr, type, name)                                          \
  ((type *)(((char *)(ptr)) - offsetof(type, name)))

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

void hexdump_mem(void *ptr, size_t len);
