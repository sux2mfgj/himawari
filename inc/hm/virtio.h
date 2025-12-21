#pragma once

#include <hm/pci.h>

struct virtio_device {
  struct pcie_device *pdev;
  struct virtio_pci_common_cfg *common_cfg;
  uint8_t *notify_cfg;
  uint8_t *isr_cfg;
  uint8_t *device_cfg;
  // uint8_t *pci_cfg;
};

int virtio_device_init(struct virtio_device *vdev);
int virtio_device_reset(struct virtio_device *vdev);
uint32_t virtio_read_feature(struct virtio_device *vdev);
void virtio_write_feature(struct virtio_device *vdev, uint32_t feature);
uint8_t virtio_read_status(struct virtio_device *vdev);
void virtio_write_status(struct virtio_device *vdev, uint8_t val);
