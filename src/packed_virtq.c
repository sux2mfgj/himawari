
#include <hm/mm.h>
#include <hm/print.h>
#include <hm/virtio.h>

struct packed_virtq *virtio_alloc_pvirtq(size_t nent) {

  struct packed_virtq *pvq = mm_alloc(sizeof(*pvq));
  if (!pvq)
    return NULL;

  struct pvirt_desc *vq = mm_alloc(sizeof(pvq[0]) * nent);
  if (!vq)
    return NULL;

  struct pvirtq_event_suppress *dev_sup = mm_alloc(sizeof(*dev_sup));
  if (!dev_sup)
    return NULL;

  *dev_sup = (struct pvirtq_event_suppress){0};

  struct pvirtq_event_suppress *drv_sup = mm_alloc(sizeof(*drv_sup));
  if (!drv_sup)
    return NULL;

  // Initialize driver event suppression to NOT suppress (enable all notifications)
  *drv_sup = (struct pvirtq_event_suppress){
      .counter = 0,
      .flags = 0,  // 0 = enable notifications
  };

  *pvq = (struct packed_virtq){
      .vq = vq,
      .size = nent,
      .avail_wrap_count = 1,  // VirtIO 1.1 spec: initial wrap counter is 1
      .dev_suppress = dev_sup,
      .drv_suppress = drv_sup,
  };

  return pvq;
}

static void virtio_select_queue(struct virtio_device *vdev, uint16_t idx) {
  volatile uint16_t *queue_select = &vdev->common_cfg->queue_select;

  kprintf("queue_select: addr=0x%lx, common_cfg=0x%lx, offset=%ld\n",
          (uint64_t)queue_select, (uint64_t)vdev->common_cfg,
          (uint64_t)queue_select - (uint64_t)vdev->common_cfg);

  uint16_t before = *queue_select;
  kprintf("queue_select before write: %d\n", before);

  *queue_select = idx;

  // Add memory barrier to ensure write completes
  __asm__ volatile("" ::: "memory");

  // Read back to ensure the write has taken effect and verify
  uint16_t selected = *queue_select;
  kprintf("queue_select after write: %d (expected %d)\n", selected, idx);

  if (selected != idx) {
    kprintf("ERROR: queue_select wrote %d but read back %d\n", idx, selected);
  }
}

void virtio_enable_pvirtq(struct virtio_device *vdev, uint16_t idx) {
  virtio_select_queue(vdev, idx);

  kprintf("A[%d]\n", idx);
  uint16_t selected = vdev->common_cfg->queue_select;
  kprintf("B[%d]=%d\n", idx, selected);

  // Verify queue configuration before enabling
  uint16_t size = vdev->common_cfg->queue_size;
  kprintf("C[%d]\n", idx);
  uint64_t desc = vdev->common_cfg->queue_desc;
  kprintf("D[%d]\n", idx);
  uint64_t driver = vdev->common_cfg->queue_driver;
  kprintf("E[%d]\n", idx);
  uint64_t device = vdev->common_cfg->queue_device;
  kprintf("F[%d]\n", idx);

  kprintf("virtio_enable[%d]: size=%d, desc=0x%lx, drv=0x%lx, dev=0x%lx\n",
          idx, size, desc, driver, device);

  volatile uint16_t *enable = &vdev->common_cfg->queue_enable;

  // Read the current value before enabling
  uint16_t before = *enable;
  kprintf("virtio_enable_pvirtq[%d]: enable addr=0x%lx, offset=%ld, before=%d\n",
          idx, (uint64_t)enable,
          (uint64_t)enable - (uint64_t)vdev->common_cfg, before);

  // Write 1 to enable the queue
  *enable = 1;

  // Memory barrier
  __asm__ volatile("" ::: "memory");

  uint16_t readback = *enable;
  kprintf("virtio_enable_pvirtq[%d]: wrote 1, read back %d\n", idx, readback);

  // Verify queue_select hasn't changed
  uint16_t sel_check = vdev->common_cfg->queue_select;
  kprintf("virtio_enable_pvirtq[%d]: queue_select after enable: %d\n", idx, sel_check);
}

int virtio_set_pvirtq(struct virtio_device *vdev, uint16_t idx,
                      struct packed_virtq *pvq) {

  virtio_select_queue(vdev, idx);

  kprintf("virtio_set_pvirtq[%d]: vq=0x%lx, dev_sup=0x%lx, drv_sup=0x%lx, size=%d\n",
          idx, (uint64_t)pvq->vq, (uint64_t)pvq->dev_suppress,
          (uint64_t)pvq->drv_suppress, pvq->size);

  volatile uint64_t *queue_desc = &vdev->common_cfg->queue_desc;
  *queue_desc = (uint64_t)pvq->vq;

  volatile uint16_t *queue_size = &vdev->common_cfg->queue_size;
  *queue_size = pvq->size;

  volatile uint64_t *queue_device = &vdev->common_cfg->queue_device;
  *queue_device = (uint64_t)pvq->dev_suppress;

  volatile uint64_t *queue_driver = &vdev->common_cfg->queue_driver;
  *queue_driver = (uint64_t)pvq->drv_suppress;

  // Memory barrier to ensure all writes are visible to device
  __asm__ volatile("mfence" ::: "memory");

  return 0;
}

int virtio_set_msix(struct virtio_device *vdev, uint16_t vq_idx,
                    uint16_t msix_vector) {

  virtio_select_queue(vdev, vq_idx);

  volatile uint16_t *queue_msix_vector = &vdev->common_cfg->queue_msix_vector;
  *queue_msix_vector = msix_vector;

  // Memory barrier to ensure write completes
  __asm__ volatile("mfence" ::: "memory");

  // Read back to verify
  uint16_t readback = *queue_msix_vector;

  kprintf("virtio_set_msix[%d]: wrote %d, readback %d\n", vq_idx, msix_vector, readback);

  if (readback == 0xffff) {
    kprintf("virtio_set_msix[%d]: ERROR - device returned NO_VECTOR (0xffff)\n", vq_idx);
    return -1;
  }

  if (readback != msix_vector) {
    kprintf("virtio_set_msix[%d]: WARNING - readback mismatch (wrote %d, got %d)\n",
            vq_idx, msix_vector, readback);
  }

  return 0;
}
