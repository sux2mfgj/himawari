#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool memcmp(void *a, void *b, size_t size);
void memset(void *a, uint8_t byte, size_t size);
void memcpy(void *dst, void *src, size_t size);
