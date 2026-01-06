#include <hm/apic.h>
#include <hm/device.h>
#include <hm/int.h>
#include <hm/mm.h>
#include <hm/module.h>
#include <hm/msix.h>
#include <hm/net_if.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/utils.h>
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
  struct net_if nif;
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
  // num_buffers field is only present if VIRTIO_NET_F_MRG_RXBUF is negotiated
  // Since we don't negotiate that feature, header is only 10 bytes
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

  // All buffers are now available, so last_avail_idx should be at size (will
  // wrap to 0) But actually, we've filled indices 0..size-1, so next available
  // is 0 (with potential wrap) Since we started with idx=0 and filled all, the
  // next idx to fill is 0 again But we need to track that we've made size
  // buffers available
  pvq->last_avail_idx = 0; // Next buffer to add (will wrap)
  // Note: avail_wrap_count should NOT be toggled here as we haven't wrapped yet

  VNET_LOG("Filled %d RX buffers, wrap_count=%d, flags=0x%x", pvq->size,
           pvq->avail_wrap_count,
           (uint32_t)(VIRTQ_DESC_F_WRITE | avail_flag | used_flag));

  return 0;
}

static int vnet_config_irq_handler(uint16_t irqn, struct context *_ctx,
                                   void *ctx) {
  struct virtio_net *vnet = (struct virtio_net *)ctx;
  VNET_DEBUG("Config interrupt fired");

  return 0;
}

static int vnet_tx_irq_handler(uint16_t irqn, struct context *_ctx, void *ctx) {
  struct virtio_net *vnet = (struct virtio_net *)ctx;
  VNET_DEBUG("TX interrupt fired! irqn=%d", irqn);

  // TODO: Process TX completion (free buffers, etc.)

  // Send EOI to re-enable interrupts
  local_apic_eoi(NULL);
  return 0;
}

static int vnet_rx_irq_handler(uint16_t irqn, struct context *_ctx, void *ctx) {

  struct virtio_net *vnet = (struct virtio_net *)ctx;

  struct packed_virtq *rxq = vnet->rxq;
  struct virtio_device *vdev = &vnet->vdev;

  // Determine expected AVAIL/USED flags for available (not-yet-used)
  // descriptors When used_wrap_count=1: used desc has AVAIL=1, USED=1 When
  // used_wrap_count=0: used desc has AVAIL=0, USED=0
  uint16_t expected_used_avail = rxq->used_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
  uint16_t expected_used_used = rxq->used_wrap_count ? VIRTQ_DESC_F_USED : 0;
  uint16_t expected_used_flags = expected_used_avail | expected_used_used;

  int packets_received = 0;

  // Process used buffers starting from last_used_idx
  // Stop when we encounter a descriptor that hasn't been used yet
  while (packets_received < rxq->size) {
    uint16_t idx = rxq->last_used_idx;
    uint16_t flags = rxq->vq[idx].flags;

    // Check if this descriptor has been used by the device
    // Expected: both AVAIL and USED bits match used_wrap_count
    uint16_t flags_masked = flags & (VIRTQ_DESC_F_AVAIL | VIRTQ_DESC_F_USED);
    if (flags_masked != expected_used_flags) {
      // Descriptor not yet used, stop processing
      break;
    }

    uint64_t buf_addr = rxq->vq[idx].addr;
    uint32_t buf_len = rxq->vq[idx].size;

    // Parse virtio-net header (10 bytes without VIRTIO_NET_F_MRG_RXBUF)
    struct virtio_net_hdr *hdr = (struct virtio_net_hdr *)buf_addr;
    uint8_t *packet_data = (uint8_t *)(buf_addr + sizeof(*hdr));
    uint32_t packet_len = buf_len - sizeof(*hdr);

    // Pass packet to network stack
    netif_receive_packet(&vnet->nif, packet_data, packet_len);

    VNET_LOG("RX packet #%d (idx=%d): total_len=%d, hdr.flags=0x%02x, "
             "hdr.gso_type=0x%02x",
             packets_received, idx, buf_len, hdr->flags, hdr->gso_type);

    packets_received++;

    // Increment last_used_idx and wrap if necessary
    rxq->last_used_idx++;
    if (rxq->last_used_idx >= rxq->size) {
      rxq->last_used_idx = 0;
      rxq->used_wrap_count ^= 1; // Toggle wrap counter
      // Update expected flags for next iteration
      expected_used_avail = rxq->used_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
      expected_used_used = rxq->used_wrap_count ? VIRTQ_DESC_F_USED : 0;
      expected_used_flags = expected_used_avail | expected_used_used;
      VNET_DEBUG("Used wrap toggled to %d at idx wrap", rxq->used_wrap_count);
    }

    // Restore buffer size to original
    rxq->vq[idx].size = 0x1000;

    // Full memory fence to ensure all previous writes are visible to device
    __asm__ volatile("mfence" ::: "memory");

    // Set AVAIL and USED flags based on avail_wrap_count
    // This makes the descriptor available again with the current wrap counter
    uint16_t new_avail = rxq->avail_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
    uint16_t new_used = rxq->avail_wrap_count ? 0 : VIRTQ_DESC_F_USED;
    uint16_t new_flags = VIRTQ_DESC_F_WRITE | new_avail | new_used;
    rxq->vq[idx].flags = new_flags;

    // Another fence after updating flags
    __asm__ volatile("mfence" ::: "memory");

    // Update last_avail_idx to track this refilled buffer
    // In packed virtqueues, we refill in place, so last_avail moves forward
    rxq->last_avail_idx++;
    if (rxq->last_avail_idx >= rxq->size) {
      rxq->last_avail_idx = 0;
      rxq->avail_wrap_count ^= 1; // Toggle wrap counter
      VNET_DEBUG("Avail wrap toggled to %d", rxq->avail_wrap_count);
    }
  }

  if (packets_received > 0) {
    VNET_LOG("Processed %d RX packets, refilling buffers", packets_received);

    // Re-enable interrupts by clearing driver suppression flags
    // This tells the device we want to be notified about new packets
    rxq->drv_suppress->flags = 0;
    __asm__ volatile("mfence" ::: "memory");

    // Final notification after all buffers refilled
    virtio_notify_queue(vdev, 0); // Notify device of refilled buffers

  } else {
    VNET_DEBUG("RX interrupt but no packets found (spurious?)");
  }

  // Send EOI at the end, after all processing is complete
  local_apic_eoi(NULL);

  // Also check if any OTHER ISR bits are set
  int other_isr_set = 0;
  for (int v = 0; v < 256; v++) {
    if (v != irqn && local_apic_read_isr(NULL, v)) {
      VNET_DEBUG("WARNING: ISR bit %d is also set!", v);
      other_isr_set = 1;
    }
  }

  return 0;
}

static int vnet_tx_packet(struct net_if *nif, struct packet_t *pkt) {
  struct virtio_net *vnet = container_of(nif, struct virtio_net, nif);
  struct virtio_device *vdev = &vnet->vdev;
  VNET_DEBUG("tx packet");

  struct packed_virtq *txq = vnet->txq;

  uint16_t avail_flag = txq->avail_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
  uint16_t used_flag = txq->avail_wrap_count ? 0 : VIRTQ_DESC_F_USED;

  VNET_DEBUG("tx packet");
  uint16_t idx = txq->last_used_idx;

  struct virtio_net_hdr *vnet_hdr = (struct virtio_net_hdr *)pkt->buf;
  memset(vnet_hdr, 0x00, sizeof(*vnet_hdr));
  VNET_DEBUG("tx packet");

  // Use data_len if set, otherwise fall back to buf_size
  size_t tx_len = pkt->data_len > 0 ? pkt->data_len : pkt->buf_size;

  txq->vq[idx].addr = (uint64_t)pkt->buf;
  txq->vq[idx].size = tx_len;
  txq->vq[idx].flags = avail_flag | used_flag;

  txq->last_used_idx++;

  virtio_notify_queue(vdev, 1); // Notify RX queue

  VNET_DEBUG("tx packet");

  return 0;
}

static struct net_if_ops vnet_netif_ops = {
    .tx_packet = vnet_tx_packet,
};

static int vnet_obtain_mac_addr(struct virtio_net *vnet, mac_addr_t mac_addr) {

  memcpy(mac_addr, (void *)vnet->device_config->mac, sizeof(uint8_t) * 6);
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

  // Enable PCI bus mastering before enabling MSI-X
  pci_enable_bus_master(pdev);

  // Enable MSI-X first
  msix_enable(pdev);

  uint16_t config_irq;
  register_irq_handler(vnet_config_irq_handler, &config_irq, vnet);
  uint16_t rx_irq;
  register_irq_handler(vnet_rx_irq_handler, &rx_irq, vnet);
  uint16_t tx_irq;
  register_irq_handler(vnet_tx_irq_handler, &tx_irq, vnet);

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

  // Also check TX queue
  __asm__ volatile("" ::: "memory");

  netif_register(&vnet->nif, &vnet_netif_ops, sizeof(struct virtio_net_hdr));

  mac_addr_t mac;
  vnet_obtain_mac_addr(vnet, mac);
  VNET_LOG("Mac addr: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);
  netif_set_mac_addr(&vnet->nif, mac);

  netif_set_ipv4_addr(&vnet->nif, 0x0a000202);

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
