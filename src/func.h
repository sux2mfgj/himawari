
#ifndef _FUNC_H_
#define _FUNC_H_

#include "task.h"

extern void io_hlt(void);
extern void io_cli(void);
extern void io_sti(void);

extern void write_mem8(int addr, int data);

extern int io_in8(int port);
extern int io_in16(int port);
extern int io_in32(int port);

extern void io_out8(int port, int data);
extern void io_out16(int port, int data);
extern void io_out32(int port, int data);

extern int io_load_eflags(void);
extern void io_store_eflags(int eflags);

extern void load_gdtr(int a, int b);
extern void load_idtr(int a, int b);

extern void asm_inthandler21(int *esp);
extern void asm_timer_inthandler(int *esp);
extern void asm_fault_inthandler(int *esp);
extern void asm_fault_inthandler2(int *esp);
extern void asm_page_fault_handler(int *eps);

extern int load_cr0(void);
extern void store_cr0(int cr0);

extern int memtest_sub(unsigned int start, unsigned int end);

extern void set_page_directory(uintptr_t page_directory_addr);
extern void enable_paging(void);

extern void set_esp(uintptr_t *esp);
#endif
