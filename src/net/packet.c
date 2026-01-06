#include <hm/mm.h>
#include <hm/packet.h>

int packet_buf_alloc(struct net_if *nif, size_t l3_size,
                     struct packet_t **ret_pkt) {

  struct packet_t *pkt;

  pkt = mm_alloc(sizeof(*pkt));
  if (!pkt)
    return -1;

  size_t size = netif_trasport_offset(nif) + l3_size;

  uint8_t *buf = mm_alloc(size);
  if (!buf)
    return -1;

  *pkt = (struct packet_t){
      .buf = buf,
      .buf_size = size,
      .data_len = 0,  // Will be set when data is written
      .ref_count = 1,
  };

  netif_init_packet_offset(nif, pkt);

  *ret_pkt = pkt;

  return 0;
}

int packet_buf_free(struct packet_t *pkt) {
  if (!pkt && !pkt->ref_count)
    return -1;

  pkt->ref_count--;

  if (pkt->ref_count)
    return 0;

  mm_free(pkt->buf);
  mm_free(pkt);

  return 0;
}
