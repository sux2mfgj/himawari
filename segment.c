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
        set_gatedesc(idt + i, 0, 0, 0, 0, 0, 0);
    }
    load_idtr(0x7ff, IDT_ADDR);

    set_gatedesc(
            idt + 0x21,
            (int)asm_inthandler21,
            1*8,
            GATE_TYPE_32BIT_INT,
            0,
            PRIVILEGE_LEVEL_OS,
            PRESENT
            );

    set_gatedesc(
            idt + 0x20,
            (int)asm_timer_inthandler,
            1*8,
            GATE_TYPE_32BIT_INT,
            0,
            PRIVILEGE_LEVEL_OS,
            PRESENT
            );


    return;

}

void set_segmdesc(
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

void set_gatedesc(
        struct GATE_DISCRIPTOR *gd,
        unsigned offset,
        unsigned selector,
        unsigned char gate_type,
        unsigned char storage_segment,
        unsigned char descriptor_privilege_level,
        unsigned char present)
{
        gd->offset_low = offset & 0xffff;
        gd->selector = selector;
        gd->gate_type = gate_type & 0xf;
        gd->storage_segment = storage_segment & 0x1;
        gd->descriptor_privilege_level = descriptor_privilege_level & 0x3;
        gd->present = present & 0x1;
        gd->offset_high = (offset >> 16) & 0xffff;
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

    io_out8(PIC_MASTER_DATA_PORT, 0xf8); //1111 1000
    io_out8(PIC_SLAVE_DATA_PORT, 0xff);  //1111 1111

    return;
}

inline void init_pit(void)
{
    set_pit_count(100, PIT_CONTROL_WORD_SC_COUNTER0, PIT_CONTROL_WORD_MODE_SQARE);

}

void set_pit_count(int freq, unsigned char counter, unsigned char mode)
{
    unsigned short count;
    unsigned char command;

    count = (unsigned short)(PIT_CH0_CLK / freq);

    command = mode | PIT_CONTROL_WORD_RL_LSB_MSB | counter;

    io_out8(PIT_PORT_CONTROL_WORD, command);

    io_out8(PIT_PORT_COUNTER0, (unsigned char)(count & 0xff));
    io_out8(PIT_PORT_COUNTER0, (unsigned char)((count >> 8) & 0xff));

    return;
}

void timer_interrupt(void)
{
    static unsigned int timer_tick = 0;
    io_out8(0x20, 0x20);
    io_out8(0xa0, 0x20);
    timer_tick++;
    integer_puts(timer_tick, 24);

}

void inthandler21(int *esp)
{
/*     h_puts("hello"); */
    h_puts("interrupt success");
    io_out8(0x0020, 0x61);
    io_in8(0x0060);
}

