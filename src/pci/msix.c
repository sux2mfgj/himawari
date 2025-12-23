
#include <hm/mm.h>
#include <hm/pci.h>
#include <hm/vm.h>
#include <msix.h>
#include <pcie.h>

int msix_init(struct pcie_device *pdev, struct msix_capability *cap) {

  int ret;

  uint8_t table_bar_idx = cap->table_offset & MSIX_BAR_MASK;
  uint32_t table_offset = cap->table_offset & MSIX_OFFSET_MASK;

  uint64_t table_bar;
  ret = pci_get_bar(pdev, table_bar_idx, &table_bar, NULL);
  if (ret < 0)
    return -1;

  table_bar += table_offset;
  struct msix_table_entry *table = (struct msix_table_entry *)table_bar;
  vm_map_device_straight((uint64_t)table, 0x1000);

  uint8_t pba_bar_idx = cap->pba_offset & MSIX_BAR_MASK;
  uint32_t pba_offset = cap->pba_offset & MSIX_OFFSET_MASK;

  uint64_t pba_bar;
  ret = pci_get_bar(pdev, pba_bar_idx, &pba_bar, NULL);
  if (ret < 0)
    return -1;

  pba_bar += pba_offset;
  uint64_t *pba = (uint64_t *)pba_bar;
  vm_map_device_straight((uint64_t)pba, 0x1000);

  pdev->msix = (struct msix *)mm_alloc(sizeof(*pdev->msix));
  if (!pdev->msix)
    return -1;

  *pdev->msix = (struct msix){
      .cap = cap,
      .table = table,
      .pending_bit_array = pba,
  };

  return 0;
}

#define MSIX_MSG_CTRL_ENABLE (0x8000)
#define MSIX_MSG_CTRL_FUNC_MASK (0x4000)
#define MSIX_MSG_CTRL_TABLE_SIZE (0x7ff)

int msix_enable(struct pcie_device *pdev) {
  if (!pdev->msix)
    return -1;

  volatile uint16_t *msg_ctrl = &pdev->msix->cap->message_control;
  uint16_t ctrl = *msg_ctrl;
  ctrl &= ~MSIX_MSG_CTRL_FUNC_MASK;
  ctrl |= MSIX_MSG_CTRL_ENABLE;
  *msg_ctrl = ctrl;

  return 0;
}

int msix_set_vector(struct pcie_device *pdev, uint16_t idx, uint64_t msg_addr,
                    uint32_t msg_data) {

  if (!pdev->msix)
    return -1;

  volatile struct msix_table_entry *entry = &pdev->msix->table[idx];

  entry->message_address_lo = (uint32_t)(msg_addr & 0xffffffff);
  entry->message_address_hi = (uint32_t)(msg_addr >> 32);
  entry->message_data = msg_data;
  entry->vector_control = 0;

  return 0;
}
