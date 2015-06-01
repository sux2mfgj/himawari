#ifndef _INCLUDED_ASM_H_
#define _INCLUDED_ASM_H_
#include <stdint.h>

extern void io_hlt(void);
extern void io_cli(void);
extern void io_sti(void);

extern int io_in8(int port);
extern void io_out8(int port, int data);

extern uint32_t io_load_eflags(void);
extern void io_store_eflags(uint32_t eflags);

extern void load_gdtr(uint32_t a, uint32_t b);
extern void load_idtr(uint32_t a, int b);



#endif
