#include "hm/device.h"
#include "hm/print.h"

static struct device dev_head;
static struct device_driver drv_head;

int device_init(void) {
  dev_head.next = &dev_head;
  dev_head.prev = &dev_head;

  drv_head.next = &drv_head;
  drv_head.prev = &drv_head;

  return 0;
}

static void append_device(struct device *dev) {
  struct device *tail = dev_head.prev;

  dev->next = tail->next;
  dev->prev = tail;
  tail->next = dev;
  dev_head.prev = dev;
}

static void append_driver(struct device_driver *drv) {

  struct device_driver *tail = drv_head.prev;

  drv->next = tail->next;
  drv->prev = tail;
  tail->next = drv;
  drv_head.prev = drv;
}

int register_device(struct device *dev) {
  append_device(dev);
  return 0;
}

int register_driver(struct device_driver *drv) {

  append_driver(drv);
  return 0;
}

static int match_pcie(union match *a, union match *b) {

  return (a->pcie.device_id == b->pcie.device_id) &&
         (a->pcie.vendor_id == b->pcie.vendor_id);
}

static int match(struct device *dev, struct device_driver *drv) {

  if (dev->type != drv->type)
    return 0;

  switch (dev->type) {
  case MMIO:
    kprintf("not supported yet\n");
    break;
  case PCIE:
    if (match_pcie(&dev->match, &drv->match))
      return 1;
    break;
  }

  return 0;
}

static int match_driver(struct device *dev) {

  struct device_driver *drv = drv_head.next;

  while (drv != &drv_head) {

    if (match(dev, drv)) {
      int ret = drv->probe(dev);
      if (ret) {
        kprintf("Error on probe: %s (ret=%d)\n", dev->name, ret);
        drv = drv->next;
        continue;
      }

      return 1;
    }
    drv = drv->next;
  }

  return 0;
}

int probe_drivers(void) {

  struct device *dev = dev_head.next;
  while (dev != &dev_head) {

    if (!match_driver(dev))
      kprintf("Not found a driver for device: %s\n", dev->name);

    dev = dev->next;
  }

  return 0;
}
