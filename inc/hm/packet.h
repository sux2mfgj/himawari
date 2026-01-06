#pragma once

#include <hm/net_if.h>
#include <stddef.h>
#include <stdint.h>

struct packet_t {
  uint8_t *buf;
  size_t buf_size;
  size_t data_len;  // Actual length of data written (for TX)
  uint16_t mac_offset;
  uint16_t net_offset;
  uint16_t transport_offset;
  uint8_t ref_count;
};

int packet_buf_alloc(struct net_if *nif, size_t size,
                     struct packet_t **ret_pkt);
int packet_buf_free(struct packet_t *pkt);
