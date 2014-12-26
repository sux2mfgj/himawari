#include "segment.h"
#include "graphic.h"
#include "k_memory.h"
#include "func.h"
#include "vectors.h"
#include "trap.h"

static struct SEGMENT_DESCRIPTOR gdt[NUM_GDT];

void init_gdt(void)
{
    printf(TEXT_MODE_SCREEN_RIGHT, "gdt: 0x%x", gdt);

    // init GDT
    for (int i = 0; i < NUM_GDT; i++) {
        set_segmdesc(gdt + i, 0, 0, 0, 0, 0, 0, 0);
    }
    set_segmdesc(gdt + KERNEL_CODE_SEGMENT, 0xffffffff, 0x00000000, 0,
                 SEG_TYPE_CODE_XRC, DESC_TYPE_SEGMENT, PRIVILEGE_OS,
                 PRESENT);

    set_segmdesc(gdt + KERNEL_DATA_SEGMENT, 0xffffffff, 0x00000000, 0,
                 SEG_TYPE_DATE_REW, DESC_TYPE_SEGMENT, PRIVILEGE_OS,
                 PRESENT);

    set_segmdesc(gdt + USER_CODE_SEGMENT, 0xc0000000, 0x00000000, 0,
                 SEG_TYPE_DATE_REW, DESC_TYPE_SEGMENT, PRIVILEGE_USER,
                 PRESENT);

    set_segmdesc(gdt + USER_DATA_SEGMENT, 0xc0000000, 0x00000000, 0,
                 SEG_TYPE_DATE_REW, DESC_TYPE_SEGMENT, PRIVILEGE_USER,
                 PRESENT);

    //TODO: have to set task state segment
    load_gdtr(sizeof(struct SEGMENT_DESCRIPTOR) * (NUM_GDT - 1),
              (uintptr_t)gdt);

    return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, uint32_t limit, uint32_t base,
                  uint8_t accessed, uint8_t segment_type,
                  uint8_t descriptor_type, uint8_t descriptor_privilege_level,
                  uint8_t present)
{
    if (limit > 0xfffff) {
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

void init_pic(void)
{
    // Mask
    io_out8(PIC_MASTER_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);

    // Master
    io_out8(PIC_MASTER_CMD_STATE_PORT, PIC_MASTER_ICW1);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW2);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW3);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW4);

    // Slave
    io_out8(PIC_SLAVE_CMD_STATE_PORT, PIC_SLAVE_ICW1);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW2);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW3);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW4);

    // setting enable
    io_out8(PIC_MASTER_DATA_PORT, 0xfc);  // 1111 1100
    io_out8(PIC_SLAVE_DATA_PORT, 0xff);   // 1111 1111

    return;
}

void init_pit(void)
{
    set_pit_count(PIT_CLK_10MS, PIT_CONTROL_WORD_SC_COUNTER0,
                  PIT_CONTROL_WORD_MODE_SQARE);

    return;
}

void init_tss(void)
{
    memset(&tss0, '0', sizeof(tss_struct));
    /*     set_gatedesc(); */
}

void load_sp0(uintptr_t esp0)
{
    tss0.esp0 = esp0;
}

void set_pit_count(uint16_t count, uint8_t counter, uint8_t mode)
{
    uint8_t command;

    command = mode | PIT_CONTROL_WORD_RL_LSB_MSB | counter;

    io_out8(PIT_PORT_CONTROL_WORD, command);

    io_out8(PIT_PORT_COUNTER0, (uint8_t)(count & 0xff));
    io_out8(PIT_PORT_COUNTER0, (uint8_t)((count >> 8) & 0xff));

    return;
}

