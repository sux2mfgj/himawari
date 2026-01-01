#pragma once

#define container_of(ptr, type, name)                                          \
  ((type *)(((char *)(ptr)) - offsetof(type, name)))
