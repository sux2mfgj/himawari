#pragma once

#include <stdint.h>

#define CPUID_LEAF_MAX_LEAF 0x0
#define CPUID_LEAF_TSC_INFO 0x15
#define CPUID_LEAF_PROC_FREQ_INFO 0x16

#define CPUID_LEAF_EXT_FEAT_INFO 0x80000007

void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx,
           uint32_t *ecx, uint32_t *edx);

void cpuid_get_max_leaf(uint32_t *max_leaf);
void cpuid_get_tsc_info(uint32_t *tsc_numerator, uint32_t *tsc_denomator,
                        uint32_t *core_crystal_clock);
