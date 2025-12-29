
#include <hm/mm.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/vm.h>
#include <msix.h>
#include <pcie.h>

int msix_init(struct pcie_device *pdev, struct msix_capability *cap) {

  int ret;

  kprintf("msix_init: cap struct at 0x%lx\n", (uint64_t)cap);
  kprintf("msix_init: message_control=0x%x\n", cap->message_control);
  kprintf("msix_init: table_offset=0x%x\n", cap->table_offset);
  kprintf("msix_init: pba_offset=0x%x\n", cap->pba_offset);

  // Extract table BAR and offset using struct fields
  uint8_t table_bar_idx = cap->table_offset & MSIX_BAR_MASK;
  uint32_t table_offset = cap->table_offset & MSIX_OFFSET_MASK;

  kprintf("msix_init: table_bar_idx=%d, table_offset=0x%x\n", table_bar_idx, table_offset);

  uint64_t table_bar;
  ret = pci_get_bar(pdev, table_bar_idx, &table_bar, NULL);
  if (ret < 0)
    return -1;

  kprintf("msix_init: table_bar=0x%lx (before offset)\n", table_bar);

  table_bar += table_offset;
  struct msix_table_entry *table = (struct msix_table_entry *)table_bar;

  kprintf("msix_init: MSI-X table at 0x%lx\n", (uint64_t)table);

  vm_map_device_straight((uint64_t)table, 0x1000);

  // Extract PBA BAR and offset using struct fields
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

  volatile struct msix_capability *cap = pdev->msix->cap;

  uint16_t ctrl = cap->message_control;
  kprintf("msix_enable: cap at 0x%lx\n", (uint64_t)cap);
  kprintf("msix_enable: before ctrl=0x%x\n", ctrl);

  ctrl &= ~MSIX_MSG_CTRL_FUNC_MASK;
  ctrl |= MSIX_MSG_CTRL_ENABLE;
  cap->message_control = ctrl;

  uint16_t readback = cap->message_control;
  kprintf("msix_enable: after ctrl=0x%x (read back: 0x%x)\n", ctrl, readback);

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
  entry->vector_control = 0;  // Unmask the vector

  // Memory barrier to ensure writes complete
  __asm__ volatile("mfence" ::: "memory");

  // Read back to verify
  uint32_t addr_lo_rb = entry->message_address_lo;
  uint32_t addr_hi_rb = entry->message_address_hi;
  uint32_t data_rb = entry->message_data;
  uint32_t ctrl_rb = entry->vector_control;

  kprintf("msix_set_vector[%d]: addr=0x%x%x, data=0x%x, ctrl=0x%x\n",
          idx, entry->message_address_hi, entry->message_address_lo,
          entry->message_data, entry->vector_control);

  kprintf("msix_set_vector[%d] readback: addr=0x%x%x, data=0x%x, ctrl=0x%x\n",
          idx, addr_hi_rb, addr_lo_rb, data_rb, ctrl_rb);

  return 0;
}
