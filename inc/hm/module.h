#pragma once

/**
 * MODULE_INIT - Register a module initialization function
 * @fn: Function to be called during module initialization
 *
 * Usage:
 *   static int my_init(void) { ... }
 *   MODULE_INIT(my_init);
 */
#define MODULE_INIT(fn)                                                        \
  static int (*__init_ptr_##fn)(void)                                          \
      __attribute__((section(".module_init"), used)) = fn

/**
 * module_init_all - Initialize all registered modules
 *
 * Calls all module init functions registered via MODULE_INIT().
 * Returns 0 on success, or the first non-zero error code.
 */
int module_init_all(void);
