#include <hm/device.h>
#include <hm/mm.h>
#include <hm/module.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/virtio.h>
#include <hm/vm.h>
#include <stdint.h>
#include <virtio.h>

#define PCIE_CONFIG_SPACE_STANDARD_SIZE 0x40

// virtio-net features.
#define VIRTIO_NET_F_CSUM (0x1 << 0)
#define VIRTIO_NET_F_GUEST_CSUM (0x1 << 1)
#define VIRTIO_NET_F_CTRL_GUEST_OFFLOADS (0x1 << 2)
#define VIRTIO_NET_F_MTU (0x1 << 3)
#define VIRTIO_NET_F_MAC (0x1 << 5)
#define VIRTIO_NET_F_GUEST_TSO4 (0x1 << 7)
#define VIRTIO_NET_F_GUEST_TSO6 (0x1 << 8)
#define VIRTIO_NET_F_GUEST_ECN (0x1 << 9)
#define VIRTIO_NET_F_GUEST_UFO (0x1 << 10)
#define VIRTIO_NET_F_HOST_TSO4 (0x1 << 11)
#define VIRTIO_NET_F_HOST_TSO6 (0x1 << 12)
#define VIRTIO_NET_F_HOST_ECN (0x1 << 13)
#define VIRTIO_NET_F_HOST_UFO (0x1 << 14)
#define VIRTIO_NET_F_MRG_RXBUF (0x1 << 15)
#define VIRTIO_NET_F_STATUS (0x1 << 16)

struct virtio_net_device_config {
  uint8_t mac[6];
  uint16_t status;
  uint16_t max_virtq_pair;
  uint16_t mtu;
  uint32_t speed;
  uint8_t duplex;
  uint8_t rss_max_key_size;
  uint16_t rss_max_indirection_table_length;
  uint32_t supported_hash_types;
  uint32_t supported_tunnel_types;
} __attribute__((packed));

#define VIRITO_NET_S_LINK_UP (0x1 << 0)
#define VIRTIO_NET_S_ANNOUNCE (0x1 << 1)

struct virtio_net {
  struct virtio_device vdev;
  volatile struct virtio_net_device_config *device_config;
};

int vnet_probe(struct device *dev) {

  int ret;

  struct pcie_device *pdev = (struct pcie_device *)dev;

  struct virtio_net *vnet = (struct virtio_net *)mm_alloc(sizeof(*vnet));
  if (!vnet)
    return -1;

  *vnet = (struct virtio_net){
      .vdev.pdev = pdev,
  };

  struct virtio_device *vdev = &vnet->vdev;

  ret = virtio_device_init(vdev);
  if (ret < 0)
    return ret;

  vnet->device_config =
      (volatile struct virtio_net_device_config *)vdev->device_cfg;

  uint32_t feature = virtio_read_feature(vdev);
  kprintf("vnet: dev features: 0x%x\n", feature);

  uint32_t drv_feature =
      VIRTIO_NET_F_MTU | VIRTIO_NET_F_MAC | VIRTIO_NET_F_STATUS;

  if ((feature & drv_feature) != drv_feature)
    return -1;

  virtio_write_feature(vdev, drv_feature);

  uint8_t status = virtio_read_status(vdev);
  status |= VIRTIO_DEVICE_STATUS_FEATURE_OK;
  virtio_write_status(vdev, status);

  status = virtio_read_status(vdev);
  if (!(status & VIRTIO_DEVICE_STATUS_FEATURE_OK))
    return -1;

  status = virtio_read_status(vdev);
  status |= VIRTIO_DEVICE_STATUS_DRIVER_OK;
  virtio_write_status(vdev, status);

  return 0;
}

int vnet_init(void) {
  struct device_driver *drv = mm_alloc(sizeof(*drv));
  if (!drv)
    return -1;

  *drv = (struct device_driver){
      .type = PCIE,
      .match.pcie =
          {
              .vendor_id = 0x1AF4,
              .device_id = 0x1041,
          },
      .probe = vnet_probe,
  };

  register_driver(drv);

  return 0;
}

MODULE_INIT(vnet_init);
