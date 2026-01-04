#pragma once

#include <stddef.h>
#include <stdint.h>

struct packet_t {
  uint8_t *buf;
  size_t buf_size;
  uint16_t net_hdr_offset;
  uint16_t trasport_hdr_offset;
};

int packet_buf_alloc(size_t size, struct packet_t **ret_pkt);
