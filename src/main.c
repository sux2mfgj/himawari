#include <hm/acpi.h>
#include <hm/pmm.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/vmm.h>
#include <pvh.h>

void qemu_debugcon_putc(char c);

void kernel_cmain(struct hvm_start_info *start_info) {
  register_putc(qemu_debugcon_putc);
  kprintf("hello world\n");

  kprintf("memmap_paddr 0x%x\n", start_info->memmap_paddr);

  int ret;

  ret = pmm_init((struct hvm_memmap_table_entry *)start_info->memmap_paddr,
                 start_info->memmap_entries);
  if (ret < 0) {
    kprintf("failed to init physical memory management subsystem\n");
    goto out;
  }

  struct hvm_start_info *copied_start_info = pmm_alloc(1);
  if (!copied_start_info) {
    kprintf("");
    goto out;
  }
  memcpy(copied_start_info, start_info, 0x1000);
  start_info = copied_start_info;

  ret = vmm_init();
  if (ret < 0) {
    kprintf("failed to init virtual memory subsystem\n");
    goto out;
  }

  struct mem_block block = {
      .base = start_info->rsdp_paddr & ~(0x1000 - 1),
      .npages = 1,
  };
  vmm_map_ram_straight(&block);

  ret = acpi_init((struct rsdp_v1_t *)start_info->rsdp_paddr);
  if (ret < 0) {
    kprintf("failed to init acpi subsystem\n");
    goto out;
  }

  kprintf("success");
out:
  asm volatile("hlt");
}
