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

      uint32_t bar =
          pci_read_config_dword(config_space, PCI_CONFIG_BAR0 + vcap->bar * 4) +
          vcap->offset;

      kprintf("virtio cap: type %d, bar %d(offset 0x%x, length 0x%x: 0x%x)\n",
              vcap->cfg_type, vcap->bar, vcap->offset, vcap->length, bar);
      vm_map_device_straight((uint64_t)bar, vcap->length / 0x1000);

      switch (vcap->cfg_type) {
      case VIRTIO_PCI_CAP_COMMON_CFG: {
        vdev->common_cfg = (uint8_t *)(uintptr_t)bar;
        break;
      }
      case VIRTIO_PCI_CAP_NOTIFY_CFG:
        vdev->notify_cfg = (uint8_t *)(uintptr_t)bar;
        break;
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
      kprintf("MSI-X capability is not supported yet\n");
    }

    next = cap->cap_next;
  } while (next);

  if (!virtio_is_set_capabilities(vdev)) {
    return -1;
  }

  return 0;
}

int virtio_device_reset(struct virtio_device *vdev) {

  volatile uint8_t *device_status = &vdev->common_cfg->device_status;

  *device_status = 0;

  return 0;
}

uint32_t virtio_read_feature(struct virtio_device *vdev) {
  volatile uint32_t *device_feature = &vdev->common_cfg->device_feature;

  return *device_feature;
}

void virtio_write_feature(struct virtio_device *vdev, uint32_t feature) {
  volatile uint32_t *device_feature = &vdev->common_cfg->device_feature;
  *device_feature = feature;
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
