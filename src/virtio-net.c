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

    // For RX buffers: WRITE flag (device writes), AVAIL flag based on wrap counter
    pvq->vq[i] = (struct pvirt_desc){
        .addr = (uint64_t)buf,
        .size = 0x1000,
        .id = i,
        .flags = VIRTQ_DESC_F_WRITE | avail_flag | used_flag,
    };
  }

  // Memory barrier to ensure all descriptor writes are visible to device
  __asm__ volatile("mfence" ::: "memory");

  VNET_LOG("Filled %d RX buffers, wrap_count=%d, flags=0x%x",
           pvq->size, pvq->avail_wrap_count,
           (uint32_t)(VIRTQ_DESC_F_WRITE | avail_flag | used_flag));

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
  uint64_t required_features = VIRTIO_NET_F_MAC | VIRTIO_F_RING_PACKED;

  if ((dev_feature & required_features) != required_features) {
    VNET_LOG("missing required features: %016lx %016lx", dev_feature, required_features);
    return -1;
  }

  // Use features that the device supports
  // Include VIRTIO_F_RING_EVENT_IDX for proper interrupt handling with packed rings
  uint64_t drv_feature = dev_feature & (VIRTIO_NET_F_MTU | VIRTIO_NET_F_MAC |
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

  // Set up MSI-X vectors: vector 0 for config changes, vector 1-2 for queues
  msix_set_vector(pdev, 0, 0xfee00000, 33);  // Config MSI-X
  msix_set_vector(pdev, 1, 0xfee00000, 34);  // RX queue MSI-X
  msix_set_vector(pdev, 2, 0xfee00000, 35);  // TX queue MSI-X

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
  virtio_set_msix(vdev, 0, 1);  // RX queue uses vector 1
  virtio_set_msix(vdev, 1, 2);  // TX queue uses vector 2
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

  // Set DRIVER_OK before notifying (device only processes queues after DRIVER_OK)
  status = virtio_read_status(vdev);
  status |= VIRTIO_DEVICE_STATUS_DRIVER_OK;
  virtio_write_status(vdev, status);
  __asm__ volatile("mfence" ::: "memory");
  VNET_LOG("Driver OK status set%s", "");

  // NOW notify device that RX buffers are available
  virtio_notify_queue(vdev, 0);  // Notify RX queue
  VNET_LOG("Notified device that RX buffers are available (after DRIVER_OK)%s", "");

  // Debug: Read back queue configuration to verify
  volatile uint16_t *queue_select = &vdev->common_cfg->queue_select;
  *queue_select = 0;  // Select RX queue
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
  *queue_select = 1;  // Select TX queue
  __asm__ volatile("" ::: "memory");
  uint16_t tx_qsize = vdev->common_cfg->queue_size;
  uint16_t tx_qenable = vdev->common_cfg->queue_enable;
  uint16_t tx_qmsix = vdev->common_cfg->queue_msix_vector;
  uint64_t tx_qdesc = vdev->common_cfg->queue_desc;
  uint64_t tx_qdrv = vdev->common_cfg->queue_driver;
  uint64_t tx_qdev = vdev->common_cfg->queue_device;

  kprintf("=== TX Queue Configuration Readback ===\n");
  kprintf("  size=%d, enable=%d, msix_vec=%d\n", tx_qsize, tx_qenable, tx_qmsix);
  kprintf("  desc=0x%lx, driver=0x%lx, device=0x%lx\n", tx_qdesc, tx_qdrv, tx_qdev);
  kprintf("  Expected: desc=0x%lx, driver=0x%lx, device=0x%lx\n",
          (uint64_t)vnet->txq->vq, (uint64_t)vnet->txq->drv_suppress,
          (uint64_t)vnet->txq->dev_suppress);

  // Dump first RX descriptor in memory
  kprintf("=== First RX Descriptor Memory Dump ===\n");
  struct pvirt_desc *d0 = &vnet->rxq->vq[0];
  uint64_t *raw = (uint64_t *)d0;
  kprintf("  [0x%lx] = 0x%016lx  (addr)\n", (uint64_t)&d0->addr, raw[0]);
  kprintf("  [0x%lx] = 0x%016lx  (size:32, id:16, flags:16)\n", (uint64_t)&d0->size, raw[1]);
  kprintf("  Parsed: addr=0x%lx, size=%d, id=%d, flags=0x%x\n",
          d0->addr, d0->size, d0->id, d0->flags);

  // Dump driver event suppression
  kprintf("=== Driver Event Suppression ===\n");
  kprintf("  counter=%d, flags=0x%x\n",
          vnet->rxq->drv_suppress->counter, vnet->rxq->drv_suppress->flags);

  kprintf("\n=== Testing TX (transmit) interrupt ===\n");

  // Allocate a buffer for TX test packet
  void *tx_buf = mm_alloc(0x1000);
  if (!tx_buf) {
    kprintf("Failed to allocate TX buffer\n");
    return -1;
  }

  // Create a simple test packet (Ethernet frame)
  // Ethernet header: dst MAC (6) + src MAC (6) + ethertype (2) = 14 bytes
  uint8_t *pkt = (uint8_t *)tx_buf;

  // Destination MAC: broadcast
  pkt[0] = 0xff; pkt[1] = 0xff; pkt[2] = 0xff;
  pkt[3] = 0xff; pkt[4] = 0xff; pkt[5] = 0xff;

  // Source MAC: 52:54:00:12:34:56 (QEMU default)
  pkt[6] = 0x52; pkt[7] = 0x54; pkt[8] = 0x00;
  pkt[9] = 0x12; pkt[10] = 0x34; pkt[11] = 0x56;

  // EtherType: 0x0800 (IPv4)
  pkt[12] = 0x08; pkt[13] = 0x00;

  // Simple payload (just some test data)
  for (int i = 14; i < 64; i++) {
    pkt[i] = i;
  }

  // For virtio-net, we need a virtio_net_hdr at the beginning
  // For packed rings with modern device, header is 12 bytes
  struct {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
  } __attribute__((packed)) virtio_net_hdr = {0};

  // Shift packet data to make room for virtio_net_hdr
  for (int i = 63; i >= 0; i--) {
    pkt[12 + i] = pkt[i];
  }

  // Copy header to beginning
  uint8_t *hdr_ptr = (uint8_t *)&virtio_net_hdr;
  for (int i = 0; i < 12; i++) {
    pkt[i] = hdr_ptr[i];
  }

  int tx_pkt_len = 12 + 64; // header + ethernet frame

  // Set up TX descriptor
  // Calculate flags based on TX queue wrap counter
  uint16_t tx_avail_flag = vnet->txq->avail_wrap_count ? VIRTQ_DESC_F_AVAIL : 0;
  uint16_t tx_used_flag = vnet->txq->avail_wrap_count ? 0 : VIRTQ_DESC_F_USED;

  vnet->txq->vq[0] = (struct pvirt_desc){
      .addr = (uint64_t)tx_buf,
      .size = tx_pkt_len,
      .id = 0,
      .flags = tx_avail_flag | tx_used_flag,  // Note: TX is READ (device reads), not WRITE
  };

  __asm__ volatile("mfence" ::: "memory");

  kprintf("TX descriptor[0] setup: addr=0x%lx, size=%d, flags=0x%x\n",
          (uint64_t)tx_buf, tx_pkt_len, vnet->txq->vq[0].flags);

  // Notify TX queue
  virtio_notify_queue(vdev, 1);  // Queue 1 is TX
  kprintf("Notified TX queue\n");

  // Check TX event suppression structures
  kprintf("TX drv_suppress: counter=%d, flags=0x%x (driver tells device when to suppress)\n",
          vnet->txq->drv_suppress->counter, vnet->txq->drv_suppress->flags);
  kprintf("TX dev_suppress: counter=%d, flags=0x%x (device tells driver when to suppress)\n",
          vnet->txq->dev_suppress->counter, vnet->txq->dev_suppress->flags);

  // Dump addresses to verify they're set correctly in device
  kprintf("Event suppression addresses:\n");
  kprintf("  RX drv_suppress addr: 0x%lx\n", (uint64_t)vnet->rxq->drv_suppress);
  kprintf("  RX dev_suppress addr: 0x%lx\n", (uint64_t)vnet->rxq->dev_suppress);
  kprintf("  TX drv_suppress addr: 0x%lx\n", (uint64_t)vnet->txq->drv_suppress);
  kprintf("  TX dev_suppress addr: 0x%lx\n", (uint64_t)vnet->txq->dev_suppress);

  // Wait a bit and check if descriptor was consumed
  for (volatile int i = 0; i < 50000000; i++);

  uint16_t tx_flags_after = vnet->txq->vq[0].flags;
  kprintf("TX descriptor[0] flags after notify: 0x%x\n", tx_flags_after);

  if (tx_flags_after & VIRTQ_DESC_F_USED) {
    kprintf("*** TX DESCRIPTOR WAS CONSUMED! Device processed it! ***\n");
    kprintf("Checking for TX interrupt (vector 35)...\n");

    // Wait a bit more for interrupt to arrive
    for (volatile int i = 0; i < 10000000; i++);

    // Check APIC IRR (Interrupt Request Register) to see if interrupt is pending
    // IRR is at offset 0x200-0x270 (8 32-bit registers, each covering 32 vectors)
    // Vector 35 is in IRR1 (vectors 32-63), bit 3 (35-32=3)
    volatile uint32_t *irr1 = (volatile uint32_t *)0xfee00210UL;  // IRR for vectors 32-63
    uint32_t irr1_val = *irr1;
    kprintf("APIC IRR1 (vectors 32-63): 0x%x\n", irr1_val);

    if (irr1_val & (1 << 3)) {
      kprintf("*** Vector 35 is PENDING in IRR! Interrupt reached APIC but not delivered! ***\n");
    } else if (irr1_val & (1 << 2)) {
      kprintf("*** Vector 34 is PENDING in IRR! ***\n");
    } else if (irr1_val & (1 << 1)) {
      kprintf("*** Vector 33 is PENDING in IRR! ***\n");
    } else {
      kprintf("No MSI-X vectors pending in IRR - interrupt never reached APIC\n");
    }

    kprintf("If no 'virtio-net TX interrupt' message appeared, MSI-X is not triggering\n");
  } else {
    kprintf("TX descriptor was NOT consumed (flags unchanged)\n");
  }

  kprintf("\nDriver initialization complete\n");

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
