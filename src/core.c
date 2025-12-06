#include <hm/core.h>
#include <hm/pmm.h>
#include <hm/print.h>
#include <stdbool.h>

static uint64_t boot_cid;
static bool initilized = false;

struct core_info {
  uint64_t ncores;
  uint64_t cids[];
};

static struct core_info *core_info;

int core_register_app_core_id(uint64_t core_id) {

  if (core_id == core_info->cids[0])
    return 0;

  core_info->cids[core_info->ncores] = core_id;
  core_info->ncores++;

  kprintf("regiseter a new core: %d (ncores %d)\n", core_id, core_info->ncores);

  // start the core.

  return 0;
}

int core_init(uint64_t boot_core_id) {

  if (initilized)
    return -1;

  boot_cid = boot_core_id;

  core_info = pmm_alloc(0x1000);
  if (!core_info)
    return -1;

  *core_info = (struct core_info){
      .ncores = 1,
  };

  core_info->cids[0] = boot_core_id;

  kprintf("regiseter a boot core: %d\n", boot_core_id);

  initilized = true;

  return 0;
}
