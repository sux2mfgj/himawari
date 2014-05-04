#include"memory.h"
#include"func.h"
#include"graphic.h"


unsigned int memtest(
        unsigned int start,
        unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0){
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return i;

}

/* unsigned int memtest_sub( */
/*         unsigned int start, */
/*         unsigned int end) */
/* { */
/*     unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa; */
/*     for(i = start; i <= end; i += 4){ */
/*         p = (unsigned int *)i; */
/*         old = *p; */
/*         *p = pat0; */
/*         *p ^= 0xffffffff; */
/*         if(*p != pat1){ */

/* not_memory: */
/*             *p = old; */
/*             break; */
/*         } */

/*         *p ^= 0xffffffff; */
/*         if(*p != pat0){ */
/*             goto not_memory; */
/*         } */
/*         *p = old; */
/*     } */

/*     return i; */

/* } */

void init_memory(void)
{
    integer_puts(memtest(0x00400000, 0xbfffffff) / (1024 * 1024), 20);
    integer_puts(get_size_of_kernel(), 19);
    integer_puts((unsigned int)&_bss_end, 18);
    integer_puts((unsigned int)&_text_start, 17);
}

unsigned int get_size_of_kernel()
{
/*     return (); */
    return (&_bss_end - &_text_start);
}

