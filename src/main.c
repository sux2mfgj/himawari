#include <hm/acpi.h>
#include <hm/int.h>
#include <hm/pmm.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/vmm.h>
#include <pvh.h>

void qemu_debugcon_putc(char c);

void kernel_cmain(struct hvm_start_info *start_info) {
  register_putc(qemu_debugcon_putc);
  kprintf("hello world\n");

  int ret;

  ret = int_init();
  if (ret < 0) {
    kprintf("failed to setup exception handlers\n");
    goto fail;
  }

  ret = pmm_init((struct hvm_memmap_table_entry *)start_info->memmap_paddr,
                 start_info->memmap_entries);
  if (ret < 0) {
    kprintf("failed to init physical memory management subsystem\n");
    goto fail;
  }

  struct hvm_start_info *copied_start_info = pmm_alloc(1);
  if (!copied_start_info) {
    kprintf("");
    goto fail;
  }
  memcpy(copied_start_info, start_info, 0x1000);
  start_info = copied_start_info;

  ret = vmm_init();
  if (ret < 0) {
    kprintf("failed to init virtual memory subsystem\n");
    goto fail;
  }

  struct mem_block block = {
      .base = start_info->rsdp_paddr & ~(0x1000 - 1),
      .npages = 1,
  };
  vmm_map_ram_straight(&block);

  ret = acpi_init((struct rsdp_v1_t *)start_info->rsdp_paddr);
  if (ret < 0) {
    kprintf("failed to init acpi subsystem\n");
    goto fail;
  }

  kprintf("success\n");
  goto out;

fail:
  kprintf("fail\n");
out:
  asm volatile("hlt");
}
