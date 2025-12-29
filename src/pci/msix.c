
#include <hm/mm.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/vm.h>
#include <msix.h>
#include <pcie.h>

int msix_init(struct pcie_device *pdev, struct msix_capability *cap) {

  int ret;

  kprintf("msix_init: cap struct at 0x%lx\n", (uint64_t)cap);

  // Read directly from config space to avoid struct alignment issues
  uint8_t *cap_ptr = (uint8_t *)cap;
  uint16_t msg_ctrl_direct = *(uint16_t *)(cap_ptr + 2);
  uint32_t table_off_direct = *(uint32_t *)(cap_ptr + 4);
  uint32_t pba_off_direct = *(uint32_t *)(cap_ptr + 8);

  kprintf("msix_init: Direct read: msg_ctrl=0x%x, table_off=0x%x, pba_off=0x%x\n",
          msg_ctrl_direct, table_off_direct, pba_off_direct);

  kprintf("msix_init: cap->table_offset raw = 0x%x\n", cap->table_offset);
  kprintf("msix_init: cap->pba_offset raw = 0x%x\n", cap->pba_offset);

  // Use direct reads to avoid struct alignment issues
  uint8_t table_bar_idx = table_off_direct & MSIX_BAR_MASK;
  uint32_t table_offset = table_off_direct & MSIX_OFFSET_MASK;

  kprintf("msix_init: table_bar_idx=%d, table_offset=0x%x\n", table_bar_idx, table_offset);

  // DEBUG: Print all BARs
  for (int i = 0; i < 6; i++) {
    uint32_t bar_val = pci_read_config_dword(pdev->config_space, PCI_CONFIG_BAR0 + i*4);
    kprintf("msix_init: BAR%d raw = 0x%x\n", i, bar_val);
  }

  uint64_t table_bar;
  ret = pci_get_bar(pdev, table_bar_idx, &table_bar, NULL);
  if (ret < 0)
    return -1;

  kprintf("msix_init: table_bar=0x%lx (before offset)\n", table_bar);

  table_bar += table_offset;
  struct msix_table_entry *table = (struct msix_table_entry *)table_bar;

  kprintf("msix_init: MSI-X table at 0x%lx\n", (uint64_t)table);

  vm_map_device_straight((uint64_t)table, 0x1000);

  // Use direct read for PBA as well
  uint8_t pba_bar_idx = pba_off_direct & MSIX_BAR_MASK;
  uint32_t pba_offset = pba_off_direct & MSIX_OFFSET_MASK;

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

  // Access message_control register directly via config space to avoid alignment issues
  uint8_t *cap_base = (uint8_t *)pdev->msix->cap;
  volatile uint16_t *msg_ctrl = (volatile uint16_t *)(cap_base + 2); // offset 2 in capability structure

  uint16_t ctrl = *msg_ctrl;
  kprintf("msix_enable: cap at 0x%lx, msg_ctrl at 0x%lx\n",
          (uint64_t)pdev->msix->cap, (uint64_t)msg_ctrl);
  kprintf("msix_enable: before ctrl=0x%x\n", ctrl);

  ctrl &= ~MSIX_MSG_CTRL_FUNC_MASK;
  ctrl |= MSIX_MSG_CTRL_ENABLE;
  *msg_ctrl = ctrl;

  uint16_t readback = *msg_ctrl;
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
