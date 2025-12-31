#include <hm/module.h>
#include <hm/net_if.h>

int net_receive_packet(struct net_if *nif, uint8_t *packet, size_t length) {

  return 0;
}

static struct net_if net_if_head;

static void append_netif(struct net_if *nif) {
  struct net_if *tail = net_if_head.prev;

  nif->next = tail->next;
  nif->prev = tail;
  tail->next = nif;
  net_if_head.prev = nif;
}

int netif_register(struct net_if *nif) {
  append_netif(nif);

  return 0;
}

static int netif_init(void) {

  net_if_head.next = &net_if_head;
  net_if_head.prev = &net_if_head;

  return 0;
}

MODULE_INIT(netif_init);
