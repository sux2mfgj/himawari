#include "cpuid.h"


void cpuid(uint32_t eax, uint32_t *edx)
{
    __asm__ volatile("cpuid" : "=d"(*edx) : "a"(eax));
}
