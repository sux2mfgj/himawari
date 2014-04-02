#ifndef _INCLUDED_SEGMENT_H_
#define _INCLUDED_SEGMENT_H_


struct SEGMENT_DESCRIPTOR{
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DISCRIPTOR{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DISCRIPTOR *gd, int offset, int selector, int ar);
void init_pic(void);
extern void load_gdtr(int limit, int addr);
extern void load_idtr(int limit, int addr);
extern void io_out8(int port, int data);
void io_hlt(void);

void asm_inthandler21(int *esp);

#define AR_INTGATE32 0x008e

#define PIC_MASTER_CMD_STATE_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_STATE_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_MASTER_ICW1 0x11
#define PIC_MASTER_ICW2 0x20
#define PIC_MASTER_ICW3 0x04
#define PIC_MASTER_ICW4 0x01

#define PIC_SLAVE_ICW1 PIC_MASTER_ICW1
#define PIC_SLAVE_ICW2 0x28
#define PIC_SLAVE_ICW3 0x02
#define PIC_SLAVE_ICW4 PIC_MASTER_ICW4

#define PIC_IMR_MASK_IRQ0 0x01
#define PIC_IMR_MASK_IRQ1 0x02
#define PIC_IMR_MASK_IRQ2 0x04
#define PIC_IMR_MASK_IRQ3 0x08
#define PIC_IMR_MASK_IRQ4 0x10
#define PIC_IMR_MASK_IRQ5 0x20
#define PIC_IMR_MASK_IRQ6 0x40
#define PIC_IMR_MASK_IRQ7 0x80
#define PIC_IMR_MASK_IRQ_ALL 0xff


#endif
