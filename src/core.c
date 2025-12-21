#include <hm/apic.h>
#include <hm/core.h>
#include <hm/mm.h>
#include <hm/print.h>
#include <hm/string.h>
#include <stdbool.h>

void _boot16(void);

static uint64_t boot_cid;
static bool initilized = false;

struct core_info {
  uint64_t ncores;
  uint64_t cids[];
};

static struct core_info *core_info;

int core_register_app_core_id(uint64_t core_id) {

  if (core_id == boot_cid)
    return 0;

  core_info->cids[core_info->ncores] = core_id;
  core_info->ncores++;

  kprintf("regiseter a new core: %d (ncores %d)\n", core_id, core_info->ncores);

  return 0;
}

int core_init(uint64_t boot_core_id) {

  if (initilized)
    return -1;

  boot_cid = boot_core_id;

  core_info = mm_alloc(0x1000);
  if (!core_info)
    return -1;

  *core_info = (struct core_info){
      .ncores = 1,
  };

  core_info->cids[0] = boot_cid;

  kprintf("regiseter a boot core: %d\n", boot_cid);

  initilized = true;

  return 0;
}

void ap_main(void) {
  // TODO: get actual APIC ID and perform AP-specific initialization

  // For now, just halt
  while (1) {
    __asm__ volatile("hlt");
  }
}

extern char _ap_boot_code_start[], _ap_boot_code_end[];

void delay(int ms);
int core_start_aps(void) {

  // Copy AP boot code to physical address 0x10000
  // The AP boot code is stored in the kernel .text section but needs
  // to be at physical address 0x10000 for SIPI to work
  size_t ap_code_size = _ap_boot_code_end - _ap_boot_code_start;
  memcpy((void *)0x10000, _ap_boot_code_start, ap_code_size);

  for (int i = 0; i < core_info->ncores; i++) {
    uint64_t cid = core_info->cids[i];
    if (boot_cid == cid)
      continue;

    // AP boot code has been copied to physical address 0x10000
    local_apic_start_ap(cid, (void *)0x10000);

    // Wait for AP to complete boot sequence (real mode → protected → long mode)
    delay(200);
  }

  return 0;
}
