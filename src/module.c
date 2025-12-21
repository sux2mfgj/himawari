#include <hm/linker.h>
#include <hm/module.h>
#include <hm/print.h>

/**
 * module_init_all - Initialize all registered modules
 *
 * Iterates through the module init function pointer array
 * (bounded by __module_init_start and __module_init_end)
 * and calls each initialization function.
 *
 * Returns 0 on success, or the first non-zero error code.
 */
int module_init_all(void) {
  typedef int (*init_fn_t)(void);

  init_fn_t *init_fn = (init_fn_t *)&__module_init_start;
  init_fn_t *init_end = (init_fn_t *)&__module_init_end;

  while (init_fn < init_end) {
    int ret = (*init_fn)();
    if (ret < 0) {
      kprintf("Module init failed at %p: %d\n", init_fn, ret);
      return ret;
    }
    init_fn++;
  }

  return 0;
}
