#include <hm/string.h>

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

void memset(void *a, uint8_t byte, size_t size) {
  uint8_t *ap = (uint8_t *)a;
  for (int i = 0; i < size; i++)
    ap[i] = byte;
}

void memcpy(void *dst, void *src, size_t size) {
  uint8_t *src_p = (uint8_t *)src;
  uint8_t *dst_p = (uint8_t *)dst;

  for (int i = 0; i < size; i++)
    dst_p[i] = src_p[i];
}
