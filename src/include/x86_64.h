#pragma once

#define MSR_IA32_EFER 0x0c0000080

#define IA32_EFER_LMA 8


static inline void cpuid(int code, uint32_t *eax, uint32_t* edx)
{
    __asm__ volatile(
            "cpuid" : "=a"(*eax), "=d"(*edx):"0"(code):);
}
