
#include <hm/mm.h>
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

  *drv_sup = (struct pvirtq_event_suppress){0};

  *pvq = (struct packed_virtq){
      .vq = vq,
      .size = nent,
      .avail_wrap_count = 0,
      .dev_suppress = dev_sup,
      .drv_suppress = drv_sup,
  };

  return pvq;
}

static void virtio_select_queue(struct virtio_device *vdev, uint16_t idx) {
  volatile uint16_t *queue_select = &vdev->common_cfg->queue_select;
  *queue_select = idx;
}

void virtio_enable_pvirtq(struct virtio_device *vdev, uint16_t idx) {
  virtio_select_queue(vdev, idx);

  volatile uint16_t *enable = &vdev->common_cfg->queue_enable;
  *enable = true;
}

int virtio_set_pvirtq(struct virtio_device *vdev, uint16_t idx,
                      struct packed_virtq *pvq) {

  virtio_select_queue(vdev, idx);

  volatile uint64_t *queue_desc = &vdev->common_cfg->queue_desc;
  *queue_desc = (uint64_t)pvq->vq;

  volatile uint16_t *queue_size = &vdev->common_cfg->queue_size;
  *queue_size = pvq->size;

  volatile uint64_t *queue_device = &vdev->common_cfg->queue_device;
  *queue_device = (uint64_t)pvq->dev_suppress;

  volatile uint64_t *queue_driver = &vdev->common_cfg->queue_driver;
  *queue_driver = (uint64_t)pvq->drv_suppress;

  return 0;
}

int virtio_set_msix(struct virtio_device *vdev, uint16_t vq_idx,
                    uint16_t msix_vector) {

  virtio_select_queue(vdev, vq_idx);

  volatile uint16_t *queue_msix_vector = &vdev->common_cfg->queue_msix_vector;
  *queue_msix_vector = msix_vector;

  if (*queue_msix_vector == 0xffff)
    return -1;

  return 0;
}
