#include <hm/msix.h>
#include <hm/print.h>
#include <hm/virtio.h>
#include <hm/vm.h>
#include <pcie.h>
#include <virtio.h>

#define CAP_POINTER_OFFSET 0x34

static int virtio_is_set_capabilities(struct virtio_device *vdev) {
  return vdev->common_cfg && vdev->notify_cfg && vdev->isr_cfg &&
         vdev->device_cfg && vdev->device_cfg; // && vdev->pci_cfg;
}

int virtio_find_capabilities(struct virtio_device *vdev) {

  int ret;
  uint8_t *config_space = vdev->pdev->config_space;

  uint8_t next = pci_read_config_byte(config_space, CAP_POINTER_OFFSET);

  kprintf("cap pointer %d\n", next);
  if (!next)
    return -1;

  do {
    struct pcie_cap *cap = (struct pcie_cap *)(config_space + next);
    kprintf("cap id %x\n", cap->cap_id);

    if (cap->cap_id == VIRTIO_PCIE_CAP_ID) {
      struct virtio_cap *vcap = (struct virtio_cap *)cap;

      // Read BAR value and mask off the low bits (PCI BAR flags)
      // For memory BARs, bits 0-3 are flags, bit 4+ is the base address
      uint32_t bar_raw =
          pci_read_config_dword(config_space, PCI_CONFIG_BAR0 + vcap->bar * 4);
      uint32_t bar = (bar_raw & ~0xF) + vcap->offset;

      kprintf("virtio cap: type %d, bar %d(offset 0x%x, length 0x%x: 0x%x)\n",
              vcap->cfg_type, vcap->bar, vcap->offset, vcap->length, bar);
      vm_map_device_straight((uint64_t)bar, vcap->length / 0x1000);

      switch (vcap->cfg_type) {
      case VIRTIO_PCI_CAP_COMMON_CFG: {
        vdev->common_cfg = (struct virtio_pci_common_cfg *)(uintptr_t)bar;
        kprintf("common_cfg mapped at: 0x%lx\n", (uint64_t)vdev->common_cfg);
        break;
      }
      case VIRTIO_PCI_CAP_NOTIFY_CFG: {
        vdev->notify_cfg = (uint8_t *)(uintptr_t)bar;
        struct virtio_notify_cap *notify_cap = (struct virtio_notify_cap *)vcap;
        vdev->notify_off_multiplier = notify_cap->notify_off_multiplier;
        kprintf("notify_cfg at 0x%lx, multiplier=%d\n",
                (uint64_t)vdev->notify_cfg, vdev->notify_off_multiplier);
        break;
      }
      case VIRTIO_PCI_CAP_ISR_CFG:
        vdev->isr_cfg = (uint8_t *)(uintptr_t)bar;
        break;
      case VIRTIO_PCI_CAP_DEVICE_CFG:
        vdev->device_cfg = (uint8_t *)(uintptr_t)bar;
        break;
      case VIRTIO_PCI_CAP_PCI_CFG:
        // vdev->pci_cfg = (uint8_t *)(uintptr_t)bar;
        break;
      default:
        return -1;
      }
    }

    if (cap->cap_id == MSIX_PCIE_CAP_ID) {
      ret = msix_init(vdev->pdev, (struct msix_capability *)cap);
      if (ret < 0) {
        kprintf("failed to init msix\n");
        return -1;
      }
    }

    next = cap->cap_next;
  } while (next);

  ret = virtio_is_set_capabilities(vdev);
  if (!ret)
    return -1;

  return 0;
}

int virtio_device_reset(struct virtio_device *vdev) {

  volatile uint8_t *device_status = &vdev->common_cfg->device_status;

  *device_status = 0;

  return 0;
}

uint64_t virtio_read_feature(struct virtio_device *vdev) {
  volatile uint32_t *device_feature = &vdev->common_cfg->device_feature;
  volatile uint32_t *device_feature_select =
      &vdev->common_cfg->device_feature_select;

  *device_feature_select = 0;

  uint64_t feature = 0;
  feature |= (uint64_t)*device_feature;

  *device_feature_select = 1;

  feature |= (uint64_t)*device_feature << 32;

  return feature;
}

void virtio_write_feature(struct virtio_device *vdev, uint64_t feature) {
  volatile uint32_t *driver_feature = &vdev->common_cfg->driver_feature;
  volatile uint32_t *driver_feature_select =
      &vdev->common_cfg->driver_feature_select;

  *driver_feature_select = 0;
  *driver_feature = (uint32_t)(feature & 0xFFFFFFFF);

  *driver_feature_select = 1;
  *driver_feature = (uint32_t)(feature >> 32);
}

uint8_t virtio_read_status(struct virtio_device *vdev) {
  volatile uint8_t *device_status = &vdev->common_cfg->device_status;

  return *device_status;
}

void virtio_write_status(struct virtio_device *vdev, uint8_t val) {
  volatile uint8_t *device_status = &vdev->common_cfg->device_status;
  *device_status = val;
}

int virtio_device_init(struct virtio_device *vdev) {
  int ret;

  ret = virtio_find_capabilities(vdev);
  if (ret < 0)
    return ret;

  ret = virtio_device_reset(vdev);
  if (ret < 0)
    return ret;

  // step 1
  volatile uint8_t *device_status = &vdev->common_cfg->device_status;
  {
    uint8_t val = *device_status | VIRTIO_DEVICE_STATUS_ACK;
    *device_status = val;
  }

  // step2
  {
    uint8_t val = *device_status | VIRTIO_DEVICE_STATUS_DRIVER;
    *device_status = val;
  }

  return 0;
}

void virtio_notify_queue(struct virtio_device *vdev, uint16_t queue_idx) {
  // Select the queue to read its notify_off
  volatile uint16_t *queue_select = &vdev->common_cfg->queue_select;
  uint16_t prev_queue = *queue_select;
  *queue_select = queue_idx;

  // Read the queue's notify offset
  volatile uint16_t *queue_notify_off = &vdev->common_cfg->queue_notify_off;
  uint16_t notify_off = *queue_notify_off;

  // Restore previous queue selection
  *queue_select = prev_queue;

  // Calculate the notify address and write the queue index
  uint32_t offset = notify_off * vdev->notify_off_multiplier;
  volatile uint16_t *notify_addr =
      (volatile uint16_t *)(vdev->notify_cfg + offset);

  // Write the queue index to the notify address
  *notify_addr = queue_idx;

  // Memory barrier to ensure the write completes
  __asm__ volatile("mfence" ::: "memory");
}
