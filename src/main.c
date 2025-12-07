#include <hm/acpi.h>
#include <hm/int.h>
#include <hm/mm.h>
#include <hm/print.h>
#include <hm/print_setup.h>
#include <hm/string.h>
#include <hm/vm.h>
#include <pvh.h>

void qemu_debugcon_putc(char c);

static struct hvm_start_info start_info;

static void move_start_inifo(struct hvm_start_info *sinfo) {
  memcpy(&start_info, sinfo, sizeof(*sinfo));
}

void kernel_cmain(struct hvm_start_info *sinfo) {
  int ret;
  register_putc(qemu_debugcon_putc);
  kprintf("hello world\n");

  move_start_inifo(sinfo);

  ret = int_init();
  if (ret < 0) {
    kprintf("failed to setup exception handlers\n");
    goto fail;
  }

  ret = mm_early_init();
  if (ret < 0) {
    kprintf("failed to init early mm\n");
    goto fail;
  }

  ret = vm_init((struct hvm_memmap_table_entry *)start_info.memmap_paddr,
                start_info.memmap_entries);
  if (ret < 0) {
    kprintf("failed to init virtual memory subsystem\n");
    goto fail;
  }

  ret = mm_init((struct hvm_memmap_table_entry *)start_info.memmap_paddr,
                start_info.memmap_entries);
  if (ret < 0) {
    kprintf("failed to init physical memory management subsystem\n");
    goto fail;
  }

  ret = acpi_init((struct rsdp_v1_t *)start_info.rsdp_paddr);
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
