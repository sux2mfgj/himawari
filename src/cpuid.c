#include <hm/cpuid.h>

void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx,
           uint32_t *ecx, uint32_t *edx) {

  asm volatile("cpuid"
               : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
               : "a"(leaf), "c"(subleaf));
}

void cpuid_get_max_leaf(uint32_t *max_leaf) {
  uint32_t dummy[3];
  cpuid(CPUID_LEAF_MAX_LEAF, 0, max_leaf, &dummy[0], &dummy[1], &dummy[2]);
}

void cpuid_get_tsc_info(uint32_t *tsc_numerator, uint32_t *tsc_denomator,
                        uint32_t *core_crystal_clock) {

  uint32_t dummy;

  cpuid(CPUID_LEAF_TSC_INFO, 0, tsc_numerator, tsc_denomator,
        core_crystal_clock, &dummy);
}
