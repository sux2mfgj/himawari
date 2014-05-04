#ifndef _INCLUDED_MEMORY_H_
#define _INCLUDED_MEMORY_H_

#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

unsigned int memtest(unsigned int start, unsigned int end);
// unsigned int memtest_sub(unsigned int start, unsigned int end);

void init_memory(void);

extern char _bss_end, _text_start;

unsigned int get_size_of_kernel(void);

#endif

