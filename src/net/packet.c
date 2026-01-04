#include <hm/mm.h>
#include <hm/packet.h>

int packet_buf_alloc(size_t size, struct packet_t **ret_pkt) {

  struct packet_t *pkt;

  pkt = mm_alloc(sizeof(*pkt));
  if (!pkt)
    return -1;

  uint8_t *buf = mm_alloc(size);
  if (!buf)
    return -1;

  *pkt = (struct packet_t){
      .buf = buf,
      .buf_size = size,
  };

  *ret_pkt = pkt;

  return 0;
}
