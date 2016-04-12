#pragma once


#define PHYSICAL_START      0x0000000001000000
#define PAGE_OFFSET         0xffff810000000000
#define START_KERNEL_MAP    0xffffffff80000000

#define PAGE_SIZE   0x1000

#ifndef ASM_FILE

extern char _kernel_start, _kernel_end;

#endif // ASM_FILE


