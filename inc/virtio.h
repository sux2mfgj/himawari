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
  uint8_t cap_len;
  uint8_t cfg_type;
  uint8_t bar;
  uint8_t id;
  uint16_t padding;
  uint32_t offset;
  uint32_t length;
} __attribute__((packed));

struct virtio_notify_cap {
  struct virtio_cap base;
  uint32_t notify_off_multiplier;
} __attribute__((packed));

#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG 3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG 5

// common features
#define VIRTIO_F_RING_INDIRECT_DESC (0x1ULL << 28)
#define VIRTIO_F_RING_EVENT_IDX (0x1ULL << 29)
#define VIRTIO_F_VERSION_1 (0x1ULL << 32)
#define VIRTIO_F_ACCESS_PLATFORM (0x1ULL << 33)
#define VIRTIO_F_RING_PACKED (0x1ULL << 34)
#define VIRTIO_F_IN_ORDER (0x1ULL << 35)
#define VIRTIO_F_ORDER_PLATFORM (0x1ULL << 36)
#define VIRTIO_F_SR_IOV (0x1ULL << 37)
#define VIRTIO_F_NOTIFICATION_DATA (0x1ULL << 38)
#define VIRTIO_F_NOTIF_CONFIG_DATA (0x1ULL << 39)
#define VIRTIO_F_RING_RESET (0x1ULL << 40)
#define VIRTIO_F_ADMIN_VQ (0x1ULL << 41)

struct pvirt_desc {
  uint64_t addr;
  uint32_t size;
  uint16_t id;
  uint16_t flags;
} __attribute__((packed));

struct pvirtq_event_suppress {
  uint16_t counter;
  uint16_t flags;
} __attribute__((packed));

#define VIRTQ_DESC_F_NEXT (1 << 0)
#define VIRTQ_DESC_F_WRITE (1 << 1)
#define VIRTQ_DESC_F_INDIRECT (1 << 2)
#define VIRTQ_DESC_F_AVAIL (1 << 7)
#define VIRTQ_DESC_F_USED (1 << 15)
