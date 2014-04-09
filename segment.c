#include"segment.h"
#include"graphic.h"

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

    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2*8, AR_INTGATE32);

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

void inthandler21(int *esp)
{
    h_puts("hello");
    h_puts("interrupt success");
    io_hlt();
}
