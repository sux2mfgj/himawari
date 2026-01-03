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

static int netif_fill_mac_header(struct net_if *nif, mac_addr_t dst,
                                 uint16_t type, uint8_t *pkt) {
  // dst mac
  memcpy(&pkt[0], dst, sizeof(uint8_t) * 8);
  // src mac
  memcpy(&pkt[6], nif->mac_addr, sizeof(uint8_t) * 8);
  // type
  memcpy(&pkt[12], &type, sizeof(type));

  return 0;
}

int netif_tx_packet(struct net_if *nif, mac_addr_t dst, uint16_t mac_type,
                    uint8_t *payload, size_t length) {

  size_t pkt_len = 14 + max(46, length);
  uint8_t *pkt = mm_alloc(pkt_len);
  if (!pkt)
    return -1;

  netif_fill_mac_header(nif, dst, mac_type, pkt);

  uint8_t *net = pkt + 14;

  memcpy(net, payload, length);

  return nif->ops->tx_packet(nif, pkt, pkt_len);
}

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

int netif_set_ipv4_addr(struct net_if *nif, ipv4_addr_t addr) {
  if (!nif)
    return -1;

  nif->ipv4_addr = addr;

  return 0;
}

static int netif_init(void) {

  net_if_head.next = &net_if_head;
  net_if_head.prev = &net_if_head;

  return 0;
}

MODULE_INIT(netif_init);
