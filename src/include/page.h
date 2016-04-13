#pragma once


#define PHYSICAL_START      0x0000000001000000
#define PAGE_OFFSET         0xffff810000000000
#define START_KERNEL_MAP    0xffffffff80000000
//#define START_KERNEL_MAP    0xffff800000000000


#define PAGE_SIZE   0x1000

#ifndef ASM_FILE
#include <stdint.h>
extern char _kernel_start, _kernel_end;
extern uint64_t *pre_pml4, *pre_pdpt, *pre_kern_pdpt, *pre_pd, *pre_kern_pd;

enum {
    PAGE_PRESENT         =   0x0000000000000001UL,
    PAGE_READ_WRITE      =   0x0000000000000002UL,
    PAGE_USER_SUPER      =   0x0000000000000004UL,
    PAGE_WRITE_THROUGH   =   0x0000000000000008UL,
    PAGE_CACHE_DISABLE   =   0x0000000000000010UL, 
    PAGE_ACCESSED        =   0x0000000000000020UL, 
};


#endif // ASM_FILE


