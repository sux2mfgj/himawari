
#ifndef _FUNC_H_
#define _FUNC_H_

extern inline void io_hlt(void);
extern inline void io_cli(void);
extern inline void io_sti(void);

extern inline void write_mem8(int addr, int data);

extern inline void io_in8(int port);
extern inline void io_in16(int port);
extern inline void io_in32(int port);

extern inline void io_out8(int port, int data);
extern inline void io_out16(int port, int data);
extern inline void io_out32(int port, int data);

extern inline int io_load_eflags(void);
extern inline void io_store_eflags(int eflags);

extern inline void load_gdtr(int a, int b);
extern inline void load_idtr(int a, int b);

extern void asm_inthandler21(int *esp);

#endif 
