#pragma once

#include <stdint.h>

enum device_type {
  MMIO,
  PCIE,
};

union match {
  struct {
    uint16_t device_id;
    uint16_t vendor_id;
  } pcie;
  struct {
    char *id;
  } mmio;
};

struct device {
  struct device *next, *prev;
  enum device_type type;
  union match match;
  char *name;
};

struct device_driver {
  struct device_driver *next, *prev;
  enum device_type type;
  union match match;
  int (*probe)(struct device *);
};

int device_init(void);
int register_device(struct device *dev);
int register_driver(struct device_driver *drv);
int probe_drivers(void);
