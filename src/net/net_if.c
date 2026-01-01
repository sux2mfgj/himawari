#include <hm/module.h>
#include <hm/net.h>
#include <hm/net_if.h>
#include <hm/string.h>

int netif_receive_packet(struct net_if *nif, uint8_t *packet, size_t length) {
  return handle_rx_packet(nif, packet, (uint32_t)length);
}

int netif_tx_packet(struct net_if *nif, uint8_t *packet, size_t length) {}

static struct net_if net_if_head;

static void append_netif(struct net_if *nif) {
  struct net_if *tail = net_if_head.prev;

  nif->next = tail->next;
  nif->prev = tail;
  tail->next = nif;
  net_if_head.prev = nif;
}

int netif_register(struct net_if *nif, struct net_if_ops *ops) {

  *nif = (struct net_if){
      .ops = ops,
  };

  append_netif(nif);

  return 0;
}

int netif_set_mac_addr(struct net_if *nif, mac_addr_t mac_addr) {
  memcpy(nif->mac_addr, mac_addr, sizeof(uint8_t) * 6);
  return 0;
}

static int netif_init(void) {

  net_if_head.next = &net_if_head;
  net_if_head.prev = &net_if_head;

  return 0;
}

MODULE_INIT(netif_init);
