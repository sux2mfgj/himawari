#include <hm/mm.h>
#include <hm/module.h>
#include <hm/net.h>
#include <hm/net_if.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/utils.h>

int netif_receive_packet(struct net_if *nif, uint8_t *packet, size_t length) {
  kprintf("%s:%d\n", __func__, __LINE__);
  return handle_rx_packet(nif, packet, (uint32_t)length);
}

int netif_tx_packet(struct net_if *nif, struct packet_t *pkt) {
  return nif->ops->tx_packet(nif, pkt);
}

static struct net_if net_if_head;

static void append_netif(struct net_if *nif) {
  struct net_if *tail = net_if_head.prev;

  nif->next = tail->next;
  nif->prev = tail;
  tail->next = nif;
  net_if_head.prev = nif;
}

int netif_register(struct net_if *nif, struct net_if_ops *ops,
                   uint16_t mac_offset) {

  *nif = (struct net_if){
      .ops = ops,
      .mac_offset = mac_offset,
  };

  append_netif(nif);

  return 0;
}

int netif_set_mac_addr(struct net_if *nif, mac_addr_t mac_addr) {
  memcpy(nif->mac_addr, mac_addr, sizeof(uint8_t) * 6);
  return 0;
}

int netif_set_ipv4_addr(struct net_if *nif, ipv4_addr_t addr) {
  if (!nif)
    return -1;

  nif->ipv4_addr = addr;

  return 0;
}

uint16_t netif_trasport_offset(struct net_if *nif) {
  return nif->mac_offset + 14 + 20;
}

int netif_init_packet_offset(struct net_if *nif, struct packet_t *pkt) {

  pkt->mac_offset = nif->mac_offset;
  pkt->net_offset = nif->mac_offset + 14;
  pkt->transport_offset = nif->mac_offset + 14 + 20;

  return 0;
}

static int netif_init(void) {

  net_if_head.next = &net_if_head;
  net_if_head.prev = &net_if_head;

  return 0;
}

MODULE_INIT(netif_init);
