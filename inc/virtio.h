#pragma once

#include <pcie.h>

#include <stdint.h>

#define VIRTIO_DEVICE_STATUS_ACK (0x1 << 0)
#define VIRTIO_DEVICE_STATUS_DRIVER (0x1 << 1)
#define VIRTIO_DEVICE_STATUS_DRIVER_OK (0x1 << 2)
#define VIRTIO_DEVICE_STATUS_FEATURE_OK (0x1 << 3)
#define VIRTIO_DEVICE_NEED_RESET (0x1 << 6)
#define VIRTIO_DEVICE_FAILED (0x1 << 7)

struct virtio_pci_common_cfg {
  uint32_t device_feature_select;
  uint32_t device_feature;
  uint32_t driver_feature_select;
  uint32_t driver_feature;
  uint16_t config_msix_vector;
  uint16_t num_queues;
  uint8_t device_status;
  uint8_t config_generation;

  uint16_t queue_select;
  uint16_t queue_size;
  uint16_t queue_msix_vector;
  uint16_t queue_enable;
  uint16_t queue_notify_off;
  uint64_t queue_desc;
  uint64_t queue_driver;
  uint64_t queue_device;
  uint16_t queue_notif_config_data;
  uint16_t queue_reset;

  uint16_t admin_queue_index;
  uint16_t admin_queue_num;
};

struct virtio_cap {
  struct pcie_cap base;
  uint8_t cfg_type;
  uint8_t bar;
  uint8_t id;
  uint16_t padding;
  uint32_t offset;
  uint32_t length;
} __attribute__((packed));

#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG 3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG 5
