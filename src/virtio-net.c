#include <hm/device.h>
#include <hm/mm.h>
#include <hm/module.h>
#include <hm/msix.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/virtio.h>
#include <hm/vm.h>
#include <stdint.h>
#include <virtio.h>

#define VNET_LOG(fmt, ...) kprintf("vnet: " fmt "\n", __VA_ARGS__)
#define VNET_DEBUG(fmt, ...)                                                   \
  kprintf("vnet(%s:%d): " fmt "\n", __func__, __LINE__, __VA_ARGS__)

#define VQ_SIZE 16

#define PCIE_CONFIG_SPACE_STANDARD_SIZE 0x40

// virtio-net features.
#define VIRTIO_NET_F_CSUM (0x1ULL << 0)
#define VIRTIO_NET_F_GUEST_CSUM (0x1ULL << 1)
#define VIRTIO_NET_F_CTRL_GUEST_OFFLOADS (0x1 << 2)
#define VIRTIO_NET_F_MTU (0x1ULL << 3)
#define VIRTIO_NET_F_MAC (0x1ULL << 5)
#define VIRTIO_NET_F_GUEST_TSO4 (0x1ULL << 7)
#define VIRTIO_NET_F_GUEST_TSO6 (0x1ULL << 8)
#define VIRTIO_NET_F_GUEST_ECN (0x1ULL << 9)
#define VIRTIO_NET_F_GUEST_UFO (0x1ULL << 10)
#define VIRTIO_NET_F_HOST_TSO4 (0x1ULL << 11)
#define VIRTIO_NET_F_HOST_TSO6 (0x1ULL << 12)
#define VIRTIO_NET_F_HOST_ECN (0x1ULL << 13)
#define VIRTIO_NET_F_HOST_UFO (0x1ULL << 14)
#define VIRTIO_NET_F_MRG_RXBUF (0x1ULL << 15)
#define VIRTIO_NET_F_STATUS (0x1ULL << 16)

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
  struct packed_virtq *txq;
  struct packed_virtq *rxq;
};

static int fill_rx_buffers(struct virtio_net *vnet) {

  struct packed_virtq *pvq = vnet->rxq;
  for (int i = 0; i < pvq->size; i++) {

    void *buf = mm_alloc(0x1000);
    if (!buf)
      return -1;

    pvq->vq[i] = (struct pvirt_desc){
        .addr = (uint64_t)buf,
        .size = 0x1000,
        .id = i,
        .flags = VIRTQ_DESC_F_WRITE | VIRTQ_DESC_F_AVAIL,
    };
  }

  return 0;
}

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

  uint64_t dev_feature = virtio_read_feature(vdev);
  VNET_LOG("dev features: 0x%016lx", dev_feature);

  uint64_t drv_feature =
      VIRTIO_NET_F_MTU | VIRTIO_NET_F_MAC | VIRTIO_F_RING_PACKED;

  if ((dev_feature & drv_feature) != drv_feature) {
    VNET_LOG("feature mismatch: %016lx %016lx", dev_feature, drv_feature);
    return -1;
  }

  virtio_write_feature(vdev, drv_feature);

  uint8_t status = virtio_read_status(vdev);
  status |= VIRTIO_DEVICE_STATUS_FEATURE_OK;
  virtio_write_status(vdev, status);

  status = virtio_read_status(vdev);
  if (!(status & VIRTIO_DEVICE_STATUS_FEATURE_OK)) {
    VNET_LOG("feature is not ok...: %x", status);
    return -1;
  }

  struct packed_virtq *txq = virtio_alloc_pvirtq(VQ_SIZE);
  if (!txq) {
    VNET_LOG("failed to alloc packed virtqueue for %s", "tx");
    return -1;
  }

  struct packed_virtq *rxq = virtio_alloc_pvirtq(VQ_SIZE);
  if (!rxq) {
    VNET_LOG("failed to alloc packed_ virtqueue for %s", "rx");
    return -1;
  }

  vnet->txq = txq;
  vnet->rxq = rxq;

  virtio_set_pvirtq(vdev, 0, vnet->rxq);
  virtio_set_pvirtq(vdev, 1, vnet->txq);

  virtio_enable_pvirtq(vdev, 0);
  virtio_enable_pvirtq(vdev, 1);

  fill_rx_buffers(vnet);

  msix_set_vector(pdev, 0, 0xfee00000, 33);
  msix_set_vector(pdev, 1, 0xfee00000, 34);

  virtio_set_msix(vdev, 0, 0);
  virtio_set_msix(vdev, 1, 1);

  status = virtio_read_status(vdev);
  status |= VIRTIO_DEVICE_STATUS_DRIVER_OK;
  virtio_write_status(vdev, status);

  msix_enable(pdev);

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
