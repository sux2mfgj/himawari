#include <string.h>

#include <stdint.h>

bool memcmp(void *a, void *b, size_t size) {
  uint8_t *ap = (uint8_t *)a;
  uint8_t *bp = (uint8_t *)b;

  for (int i = 0; i < size; i++) {
    if (ap[i] != bp[i])
      return false;
  }

  return true;
}
