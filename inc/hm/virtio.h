#pragma once

#include <hm/pci.h>

#include <virtio.h>

#include <stdbool.h>
#include <stddef.h>

struct virtio_device {
  struct pcie_device *pdev;
  volatile struct virtio_pci_common_cfg *common_cfg;
  uint8_t *notify_cfg;
  uint32_t notify_off_multiplier;
  uint8_t *isr_cfg;
  uint8_t *device_cfg;
  // uint8_t *pci_cfg;
};

int virtio_device_init(struct virtio_device *vdev);
int virtio_device_reset(struct virtio_device *vdev);
uint64_t virtio_read_feature(struct virtio_device *vdev);
void virtio_write_feature(struct virtio_device *vdev, uint64_t feature);
uint8_t virtio_read_status(struct virtio_device *vdev);
void virtio_write_status(struct virtio_device *vdev, uint8_t val);

/*
 * Packed virtqueue
 */
struct packed_virtq {
  struct pvirt_desc *vq;
  struct pvirtq_event_suppress *dev_suppress;
  struct pvirtq_event_suppress *drv_suppress;
  uint16_t size;
  uint8_t avail_wrap_count;
  uint8_t used_wrap_count;
  uint16_t last_avail_idx;  // Next index to make available to device
  uint16_t last_used_idx;   // Next index to check for used buffers
};

struct packed_virtq *virtio_alloc_pvirtq(size_t nent);
int virtio_set_pvirtq(struct virtio_device *vdev, uint16_t idx,
                      struct packed_virtq *pvq);

void virtio_enable_pvirtq(struct virtio_device *vdev, uint16_t idx);

int virtio_set_msix(struct virtio_device *vdev, uint16_t vq_idx,
                    uint16_t msix_vector);

void virtio_notify_queue(struct virtio_device *vdev, uint16_t queue_idx);
