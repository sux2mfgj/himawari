#include"segment.h"
#include"graphic.h"

inline void init_gdtidt(void)
{
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) GDT_ADDR;
    struct GATE_DISCRIPTOR *idt = (struct GATE_DISCRIPTOR *) IDT_ADDR;
    int i;

    //init GDT
    for(i = 0; i < NUM_GDT; i++){
        set_segmdesc(gdt + i, 0, 0, 0, 0, 0, 0, 0);
    }
    set_segmdesc(
            gdt + 1, 
            0xffffffff,
            0x00000000, 
            0,
            SEG_TYPE_CODE_XRC,
            DESC_TYPE_SEGMENT,
            PRIVILEGE_LEVEL_OS,
            PRESENT
            );

    set_segmdesc(
            gdt + 2, 
            0xffffffff,
            0x00000000, 
            0,
            SEG_TYPE_DATE_REW,
            DESC_TYPE_SEGMENT,
            PRIVILEGE_LEVEL_OS,
            PRESENT
            );


    load_gdtr(0xffff, GDT_ADDR);

    //init IDT
    for(i = 0; i < NUM_IDT; i++){
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, IDT_ADDR);

    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 1*8, AR_INTGATE32);

    return;

}

static void set_segmdesc(
        struct SEGMENT_DESCRIPTOR *sd,
        unsigned int limit,
        unsigned int base,
        unsigned char accessed,
        unsigned char segment_type,
        unsigned char descriptor_type,
        unsigned char descriptor_privilege_level,
        unsigned char present
        )
{
    if(limit > 0xfffff){
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
/*     sd->access_right = ar & 0xff; */
    sd->accessed = accessed & 0x01;
    sd->segment_type = segment_type & 0x07;
    sd->descriptor_type = descriptor_type & 0x01;
    sd->descriptor_privilege_level = descriptor_privilege_level & 0x03;
    sd->present = present & 0x01;

    sd->limit_high = ((limit >> 16) & 0x0f);

    sd->available = 0;
    sd->code_segment_for_64bit = 0;
    sd->default_operand_size = 1;
    sd->granularity = 0;


    sd->base_high = (base >> 24) & 0xff;

    return;
}

static void set_gatedesc(struct GATE_DISCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;

    return;
}

inline void init_pic(void)
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

    io_out8(PIC_MASTER_DATA_PORT, 0xf9); //1111 1001
    io_out8(PIC_SLAVE_DATA_PORT, 0xff);  //1111 1111

    return;
}

void inthandler21(int *esp)
{
/*     h_puts("hello"); */
    h_puts("interrupt success");
    io_out8(0x0020, 0x61);
    io_in8(0x0060);
}
