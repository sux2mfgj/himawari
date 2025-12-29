#include <hm/apic.h>
#include <hm/device.h>
#include <hm/int.h>
#include <hm/mm.h>
#include <hm/module.h>
#include <hm/msix.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/virtio.h>
#include <hm/vm.h>
#include <stdint.h>
#include <virtio.h>

#define VNET_LOG(fmt, ...) kprintf("vnet: " fmt "\n" __VA_OPT__(, ) __VA_ARGS__)
#define VNET_DEBUG(fmt, ...)                                                   \
  kprintf("vnet(%s:%d): " fmt "\n", __func__,                                  \
          __LINE__ __VA_OPT__(, ) __VA_ARGS__)

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

struct virtio_net_hdr {
  uint8_t flags;
  uint8_t gso_type;
  uint16_t hdr_len;
  uint16_t gso_size;
  uint16_t csum_start;
  uint16_t csum_offset;
  uint16_t num_buffers;
} __attribute__((packed));

static int fill_rx_buffers(struct virtio_net *vnet) {

  struct packed_virtq *pvq = vnet->rxq;

  // Calculate the AVAIL/USED flags based on wrap counter
  // According to VirtIO 1.1 spec section 2.7:
  // - AVAIL flag matches wrap counter (bit 7)
  // - USED flag is opposite of wrap counter (bit 15)
  uint16_t avail_flag = pvq->avail_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
  uint16_t used_flag = pvq->avail_wrap_count ? 0 : VIRTQ_DESC_F_USED;

  for (int i = 0; i < pvq->size; i++) {

    void *buf = mm_alloc(0x1000);
    if (!buf)
      return -1;

    // For RX buffers: WRITE flag (device writes), AVAIL flag based on wrap
    // counter
    pvq->vq[i] = (struct pvirt_desc){
        .addr = (uint64_t)buf,
        .size = 0x1000,
        .id = i,
        .flags = VIRTQ_DESC_F_WRITE | avail_flag | used_flag,
    };
  }

  // Memory barrier to ensure all descriptor writes are visible to device
  __asm__ volatile("mfence" ::: "memory");

  VNET_LOG("Filled %d RX buffers, wrap_count=%d, flags=0x%x", pvq->size,
           pvq->avail_wrap_count,
           (uint32_t)(VIRTQ_DESC_F_WRITE | avail_flag | used_flag));

  return 0;
}

static int vnet_config_irq_handler(uint16_t irqn, struct context *ctx) {
  return 0;
}

static int vnet_tx_irq_handler(uint16_t irqn, struct context *ctx) {
  VNET_DEBUG("irqn %d", irqn);
  *(volatile uint32_t *)0xfee00080UL = 0; // Send EOI
  return 0;
}

static struct virtio_net *g_vnet = NULL; // Global pointer for RX handler

static int vnet_rx_irq_handler(uint16_t irqn, struct context *ctx) {

  VNET_DEBUG("RX interrupt fired! irqn=%d", irqn);
  local_apic_eoi(NULL);

  if (!g_vnet || !g_vnet->rxq) {
    VNET_LOG("ERROR: RX handler called but vnet not initialized");
    return 0;
  }

  struct packed_virtq *rxq = g_vnet->rxq;
  struct virtio_device *vdev = &g_vnet->vdev;

  // Determine expected USED flag based on current wrap counter
  uint16_t expected_used_flag = rxq->avail_wrap_count ? VIRTQ_DESC_F_USED : 0;

  int packets_received = 0;

  // Check all descriptors for used buffers
  for (int i = 0; i < rxq->size; i++) {
    uint16_t flags = rxq->vq[i].flags;

    // Check if device marked this descriptor as used
    if ((flags & VIRTQ_DESC_F_USED) == expected_used_flag) {
      uint64_t buf_addr = rxq->vq[i].addr;
      uint32_t buf_len = rxq->vq[i].size;

      // Parse virtio-net header
      struct virtio_net_hdr *hdr = (struct virtio_net_hdr *)buf_addr;
      uint8_t *packet_data = (uint8_t *)(buf_addr + sizeof(*hdr));
      uint32_t packet_len = buf_len - sizeof(*hdr);

      VNET_LOG(
          "RX packet #%d: total_len=%d, hdr.flags=0x%02x, hdr.gso_type=0x%02x",
          packets_received, buf_len, hdr->flags, hdr->gso_type);

      // Dump first 64 bytes of packet (Ethernet header + some payload)
      kprintf("  Packet data (first %d bytes): ",
              packet_len < 64 ? packet_len : 64);
      for (uint32_t j = 0; j < 64 && j < packet_len; j++) {
        kprintf("%02x ", packet_data[j]);
        if ((j + 1) % 16 == 0)
          kprintf("\n                                 ");
      }
      kprintf("\n");

      packets_received++;

      // Refill buffer: flip AVAIL and USED flags
      uint16_t new_avail = rxq->avail_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
      uint16_t new_used = rxq->avail_wrap_count ? 0 : VIRTQ_DESC_F_USED;
      rxq->vq[i].flags = VIRTQ_DESC_F_WRITE | new_avail | new_used;
    }
  }

  if (packets_received > 0) {
    VNET_LOG("Processed %d RX packets, refilling buffers", packets_received);
    __asm__ volatile("mfence" ::: "memory");
    virtio_notify_queue(vdev, 0); // Notify device of refilled buffers
  } else {
    VNET_DEBUG("RX interrupt but no packets found (spurious?)");
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

  // Required features
  uint64_t required_features = VIRTIO_NET_F_MAC;

  if ((dev_feature & required_features) != required_features) {
    VNET_LOG("missing required features: %016lx %016lx", dev_feature,
             required_features);
    return -1;
  }

  // Check if device supports packed rings
  if (!(dev_feature & VIRTIO_F_RING_PACKED)) {
    VNET_LOG("WARNING: Device does not support packed rings, but driver "
             "requires it");
    return -1;
  }

  // Use features that the device supports
  // Include VIRTIO_F_RING_EVENT_IDX for proper interrupt handling with packed
  // rings
  uint64_t drv_feature =
      dev_feature & (VIRTIO_NET_F_MTU | VIRTIO_NET_F_MAC |
                     VIRTIO_F_RING_PACKED | VIRTIO_F_RING_EVENT_IDX);

  VNET_LOG("negotiated features: 0x%016lx", drv_feature);
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

  // Store global pointer for RX interrupt handler
  g_vnet = vnet;

  // Enable PCI bus mastering before enabling MSI-X
  pci_enable_bus_master(pdev);

  // Enable MSI-X first
  msix_enable(pdev);

  uint16_t config_irq;
  register_irq_handler(vnet_config_irq_handler, &config_irq);
  uint16_t rx_irq;
  register_irq_handler(vnet_rx_irq_handler, &rx_irq);
  uint16_t tx_irq;
  register_irq_handler(vnet_tx_irq_handler, &tx_irq);

  VNET_LOG("IRQ numbers: config=%d, rx=%d, tx=%d", config_irq, rx_irq, tx_irq);

  // Set up MSI-X vectors: vector 0 for config changes, vector 1-2 for queues
  msix_set_vector(pdev, 0, 0xfee00000, config_irq); // Config MSI-X
  msix_set_vector(pdev, 1, 0xfee00000, rx_irq);     // RX queue MSI-X
  msix_set_vector(pdev, 2, 0xfee00000, tx_irq);     // TX queue MSI-X

  // Set virtio config MSI-X vector
  volatile uint16_t *config_msix = &vdev->common_cfg->config_msix_vector;
  *config_msix = 0;
  __asm__ volatile("mfence" ::: "memory");
  uint16_t cfg_msix_rb = *config_msix;
  VNET_LOG("config_msix_vector set to: 0x%x (readback: 0x%x)", 0, cfg_msix_rb);

  // Set up virtqueues
  virtio_set_pvirtq(vdev, 0, vnet->rxq);
  virtio_set_pvirtq(vdev, 1, vnet->txq);

  // Set queue MSI-X vectors BEFORE enabling queues
  virtio_set_msix(vdev, 0, 1); // RX queue uses vector 1
  virtio_set_msix(vdev, 1, 2); // TX queue uses vector 2
  VNET_LOG("Queue MSI-X vectors set: RX=1, TX=2%s", "");

  // Fill RX buffers before enabling the queue
  fill_rx_buffers(vnet);

  // Ensure event suppression is disabled (enable notifications)
  vnet->rxq->drv_suppress->flags = 0;
  vnet->txq->drv_suppress->flags = 0;
  __asm__ volatile("mfence" ::: "memory");
  VNET_LOG("Event suppression disabled for RX/TX queues%s", "");

  // Enable queues after MSI-X is configured
  virtio_enable_pvirtq(vdev, 0);
  virtio_enable_pvirtq(vdev, 1);
  VNET_LOG("Virtqueues enabled%s", "");

  // Set DRIVER_OK before notifying (device only processes queues after
  // DRIVER_OK)
  status = virtio_read_status(vdev);
  status |= VIRTIO_DEVICE_STATUS_DRIVER_OK;
  virtio_write_status(vdev, status);
  __asm__ volatile("mfence" ::: "memory");
  VNET_LOG("Driver OK status set%s", "");

  // NOW notify device that RX buffers are available
  virtio_notify_queue(vdev, 0); // Notify RX queue
  VNET_LOG("Notified device that RX buffers are available (after DRIVER_OK)%s",
           "");

  // Debug: Read back queue configuration to verify
  volatile uint16_t *queue_select = &vdev->common_cfg->queue_select;
  *queue_select = 0; // Select RX queue
  __asm__ volatile("" ::: "memory");

  uint16_t qsize = vdev->common_cfg->queue_size;
  uint16_t qenable = vdev->common_cfg->queue_enable;
  uint16_t qmsix = vdev->common_cfg->queue_msix_vector;
  uint64_t qdesc = vdev->common_cfg->queue_desc;
  uint64_t qdrv = vdev->common_cfg->queue_driver;
  uint64_t qdev = vdev->common_cfg->queue_device;
  uint8_t dev_status = virtio_read_status(vdev);

  kprintf("=== RX Queue Configuration Readback ===\n");
  kprintf("  size=%d, enable=%d, msix_vec=%d\n", qsize, qenable, qmsix);
  kprintf("  desc=0x%lx, driver=0x%lx, device=0x%lx\n", qdesc, qdrv, qdev);
  kprintf("  device_status=0x%x\n", dev_status);
  kprintf("  Expected: desc=0x%lx, driver=0x%lx, device=0x%lx\n",
          (uint64_t)vnet->rxq->vq, (uint64_t)vnet->rxq->drv_suppress,
          (uint64_t)vnet->rxq->dev_suppress);

  // Also check TX queue
  __asm__ volatile("" ::: "memory");

  // Check TX event suppression structures
  kprintf("TX drv_suppress: counter=%d, flags=0x%x (driver tells device when "
          "to suppress)\n",
          vnet->txq->drv_suppress->counter, vnet->txq->drv_suppress->flags);
  kprintf("TX dev_suppress: counter=%d, flags=0x%x (device tells driver when "
          "to suppress)\n",
          vnet->txq->dev_suppress->counter, vnet->txq->dev_suppress->flags);

  // Dump addresses to verify they're set correctly in device
  kprintf("Event suppression addresses:\n");
  kprintf("  RX drv_suppress addr: 0x%lx\n", (uint64_t)vnet->rxq->drv_suppress);
  kprintf("  RX dev_suppress addr: 0x%lx\n", (uint64_t)vnet->rxq->dev_suppress);
  kprintf("  TX drv_suppress addr: 0x%lx\n", (uint64_t)vnet->txq->drv_suppress);
  kprintf("  TX dev_suppress addr: 0x%lx\n", (uint64_t)vnet->txq->dev_suppress);

  kprintf("\nDriver initialization complete\n");

  return 0;
}

static struct device_driver vnet_drvs_1_1 = {
    .type = PCIE,
    .match.pcie = {.vendor_id = 0x1AF4, .device_id = 0x1041},
    .probe = vnet_probe,
};
static struct device_driver vnet_drvs_trasn = {
    .type = PCIE,
    .match.pcie = {.vendor_id = 0x1AF4, .device_id = 0x1000},
    .probe = vnet_probe,
};

int vnet_init(void) {

  register_driver(&vnet_drvs_1_1);
  register_driver(&vnet_drvs_trasn);

  return 0;
}

MODULE_INIT(vnet_init);
