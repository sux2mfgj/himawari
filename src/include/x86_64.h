#pragma once

#define MSR_IA32_EFER 0x0c0000080

#define IA32_EFER_SCE 1
#define IA32_EFER_LME 8
#define IA32_EFER_LMA 10


#ifndef ASM_FILE

enum {
    CPUID_FLAG_APIC = 0x1 << 9,
};

enum {
    IA32_APIC_BASE_MSR = 0x1B,
};

enum {
    APIC_MSR_BSP            = 0x1 << 8, 
    APIC_MSR_GLOBAL_ENABLE  = 0x1 << 11,
};

static inline void cpuid(int code, uint32_t *eax, uint32_t* edx)
{
    __asm__ volatile(
            "cpuid" : "=a"(*eax), "=d"(*edx):"0"(code):);
}

static inline void read_msr(uint32_t msr, uint32_t* eax, uint32_t* edx)
{
    __asm__ volatile(
            "rdmsr" : "=a"(*eax), "=d"(*edx) :"c"(msr));
}

static inline void write_msr(uint32_t msr, uint32_t* eax, uint32_t* edx)
{
    __asm__ volatile(
            "wrmsr" : : "a"(*eax), "d"(*eax), "c"(msr)
            );
}

static inline void outb(uint16_t port, uint8_t val)
{
//     __asm__ volatile("movb %0, %%ax", )
    __asm__ volatile("outb %%dx"::"d"(port), "a"(val));
}

static inline void inb(uint16_t port, uint8_t *val)
{
    __asm__ volatile( "mov %%dx, %%ax" : : "a"(port) );
    __asm__ volatile( "in %al, %dx" );
    __asm__ volatile( "mov %%bl, %%al" : "=b"(*val) );

}

static inline void sti(void)
{
    __asm__ volatile("sti");
}

static inline void cli(void)
{
    __asm__ volatile("cli");
}

static inline void hlt(void)
{
    __asm__ volatile("hlt");
}

#endif
