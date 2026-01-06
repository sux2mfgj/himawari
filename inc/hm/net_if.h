#pragma once

#include <hm/packet.h>
#include <stddef.h>
#include <stdint.h>

struct net_if;
struct net_if_ops {
  int (*tx_packet)(struct net_if *nif, struct packet_t *pkt);
};

typedef uint8_t mac_addr_t[6];
typedef uint32_t ipv4_addr_t;

struct arp_entry {
  ipv4_addr_t ipv4_addr;
  mac_addr_t mac_addr;
};

struct net_if {
  struct net_if *next, *prev;
  struct net_if_ops *ops;
  ipv4_addr_t ipv4_addr;
  mac_addr_t mac_addr;
  uint16_t mac_offset;
  struct arp_entry arp_table[10];
};

int netif_register(struct net_if *nif, struct net_if_ops *ops,
                   uint16_t mac_offset);
int netif_set_mac_addr(struct net_if *nif, mac_addr_t mac_addr);
int netif_set_ipv4_addr(struct net_if *nif, ipv4_addr_t addr);
int netif_receive_packet(struct net_if *nif, uint8_t *packet, size_t length);
int netif_tx_packet(struct net_if *nif, struct packet_t *pkt);
int netif_init_packet_offset(struct net_if *nif, struct packet_t *pkt);
uint16_t netif_trasport_offset(struct net_if *nif);
