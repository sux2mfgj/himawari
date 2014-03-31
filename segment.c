#include"segment.h"

/* struct SEGMENT_DESCRIPTOR{ */
/*     short limit_low, base_low; */
/*     char base_mid, access_right; */
/*     char limit_high, base_high; */
/* }; */

/* struct GATE_DISCRIPTOR{ */
/*     short offset_low, selector; */
/*     char dw_count, access_right; */
/*     short offset_high; */
/* }; */

/* void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar); */
/* void set_gatedesc(struct GATE_DISCRIPTOR *gd, int offset, int selector, int ar); */
/* extern void load_gdtr(int limit, int addr); */
/* extern void load_idtr(int limit, int addr); */
/* extern void io_out8(int port, int data); */

/* #define PIC_MASTER_CMD_STATE_PORT 0x20 */
/* #define PIC_MASTER_DATA_PORT 0x21 */
/* #define PIC_SLAVE_CMD_STATE_PORT 0xA0 */
/* #define PIC_SLAVE_DATA_PORT 0xA1 */

/* #define PIC_MASTER_ICW1 0x11 */
/* #define PIC_MASTER_ICW2 0x20 */
/* #define PIC_MASTER_ICW3 0x04 */
/* #define PIC_MASTER_ICW4 0x01 */

/* #define PIC_SLAVE_ICW1 PIC_MASTER_ICW1 */
/* #define PIC_SLAVE_ICW2 0x28 */
/* #define PIC_SLAVE_ICW3 0x02 */
/* #define PIC_SLAVE_ICW4 PIC_MASTER_ICW4 */

/* #define PIC_IMR_MASK_IRQ0 0x01 */
/* #define PIC_IMR_MASK_IRQ1 0x02 */
/* #define PIC_IMR_MASK_IRQ2 0x04 */
/* #define PIC_IMR_MASK_IRQ3 0x08 */
/* #define PIC_IMR_MASK_IRQ4 0x10 */
/* #define PIC_IMR_MASK_IRQ5 0x20 */
/* #define PIC_IMR_MASK_IRQ6 0x40 */
/* #define PIC_IMR_MASK_IRQ7 0x80 */
/* #define PIC_IMR_MASK_IRQ_ALL 0xff */

void init_gdtidt(void)
{
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
    struct GATE_DISCRIPTOR *idt = (struct GATE_DISCRIPTOR *)0x00226f800;
    int i;

    //init GDT
    for(i = 0; i < 8192; i++){
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);

    //init IDT
    for(i = 0; i < 256; i++){
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_gdtr(0x7ff, 0x0026f800);

    return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
    if(limit > 0xfffff){
        ar |= 0x80000; 
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;

    return;
}

void set_gatedesc(struct GATE_DISCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;

    return;
}

void init_pic(void)
{
    //Mask
    io_out8(PIC_MASTER_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);

    //Master
    io_out8(PIC_MASTER_CMD_STATE_PORT, PIC_MASTER_ICW1);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW2);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW3);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW4);

    //Slave
    io_out8(PIC_SLAVE_CMD_STATE_PORT, PIC_SLAVE_ICW1);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW2);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW3);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW4);

    io_out8(PIC_MASTER_DATA_PORT, 0xfb);
    io_out8(PIC_SLAVE_DATA_PORT, 0xff);

    return;
}


