
static void enable_sse(void) {
  unsigned long cr0, cr4;

  // Read CR0
  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

  // Clear EM (bit 2) and TS (bit 3), Set MP (bit 1)
  cr0 &= ~(1UL << 2);  // EM = 0 (disable FPU emulation)
  cr0 |= (1UL << 1);   // MP = 1 (monitor coprocessor)
  cr0 &= ~(1UL << 3);  // TS = 0 (clear task switched)

  // Write CR0
  __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));

  // Read CR4
  __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));

  // Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10)
  cr4 |= (1UL << 9);   // OSFXSR = 1 (enable FXSAVE/FXRSTOR)
  cr4 |= (1UL << 10);  // OSXMMEXCPT = 1 (enable SSE exceptions)

  // Write CR4
  __asm__ volatile("mov %0, %%cr4" :: "r"(cr4));
}

int arch_init(void) {
  enable_sse();

  return 0;
}
