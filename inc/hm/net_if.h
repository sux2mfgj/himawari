#pragma once

#include <stddef.h>
#include <stdint.h>

struct net_if {
  struct net_if *next, *prev;
};

int netif_register(struct net_if *nif);
int netif_receive_packet(struct net_if *nif, uint8_t *packet, size_t length);
