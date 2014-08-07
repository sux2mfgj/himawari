#ifndef _INCLUDED_SEGMENT_H_
#define _INCLUDED_SEGMENT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// #include"func.h"

#define GDT_ADDR 0x00270000
// #define IDT_ADDR 0x0026f800

#define NUM_IDT 256
#define NUM_GDT 8192

#define IDT_LIMIT NUM_IDT*8-1
// #define GDT_LIMIT

#define SEG_TYPE_DATE_R    0x00
#define SEG_TYPE_DATE_RW   0x01
#define SEG_TYPE_DATE_RE   0x02
#define SEG_TYPE_DATE_REW  0x03

#define SEG_TYPE_CODE_X    0x4
#define SEG_TYPE_CODE_XR   0x5
#define SEG_TYPE_CODE_XC   0x6
#define SEG_TYPE_CODE_XRC  0x7

#define DESC_TYPE_SYSTEM    0
#define DESC_TYPE_SEGMENT   1

#define PRIVILEGE_LEVEL_OS  0
#define PRIVILEGE_LEVEL_APP 3

#define PRESENT     1
#define NOT_PRESENT 0

#define CODE_SEGMENT_NUM 2
#define DATA_SEGMENT_NUM 3

#define GATE_TYPE_32BIT_TASK    0x5
#define GATE_TYPE_16BIT_INT     0x6
#define GATE_TYPE_16BIT_TRAP    0x7
#define GATE_TYPE_32BIT_INT     0xe
#define GATE_TYPE_32BIT_TRAP    0xf

#define PIC_MASTER_CMD_STATE_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_STATE_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_MASTER_ICW1 0x11
#define PIC_MASTER_ICW2 0x20 // use after 0x20 number interrupt descriptor table
#define PIC_MASTER_ICW3 0x04
// #define PIC_MASTER_ICW4 0x01
#define PIC_MASTER_ICW4 0x00

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

#define PIC_OCW2_EOI 0x20

#define PIT_PORT_COUNTER0       0x40
#define PIT_PORT_COUNTER1       0x41
#define PIT_PORT_COUNTER2       0x41
#define PIT_PORT_CONTROL_WORD   0x43

#define PIT_CONTROL_WORD_BCD_BINARY 0
#define PIT_CONTROL_WORD_BCD_BCD    1

#define PIT_CONTROL_WORD_MODE_TIMER         0x00
#define PIT_CONTROL_WORD_MODE_ONESHOT_TIMER 0x01
#define PIT_CONTROL_WORD_MODE_PULSE         0x02
#define PIT_CONTROL_WORD_MODE_SQARE         0x03
#define PIT_CONTROL_WORD_MODE_SOFTWARE      0x04
#define PIT_CONTROL_WORD_MODE_HARDWARE      0x05

#define PIT_CONTROL_WORD_RL_LOAD    0x0
#define PIT_CONTROL_WORD_RL_LSB     0x1  // write: LSB( Least Significant Byte )
#define PIT_CONTROL_WORD_RL_MSB     0x2  // write: MSB( Most Significant Byte )
#define PIT_CONTROL_WORD_RL_LSB_MSB 0x3  // write: LSB and MSB

#define PIT_CONTROL_WORD_SC_COUNTER0    0x00
#define PIT_CONTROL_WORD_SC_COUNTER1    0x01
#define PIT_CONTROL_WORD_SC_COUNTER2    0x02
#define PIT_CONTROL_WORD_SC_DISABLE     0x03

#define PIT_CH0_CLK     1193181.67
#define PIT_CLK_1MS     PIT_CH0_CLK / 1000
#define PIT_CLK_10MS    PIT_CH0_CLK / 100

struct SEGMENT_DESCRIPTOR
{
    uint16_t limit_low, base_low;
    uint8_t base_mid;
    unsigned accessed                   :1;
    unsigned segment_type               :3;
    unsigned descriptor_type            :1;
    unsigned descriptor_privilege_level :2;
    unsigned present                    :1;

    unsigned limit_high                 :4;

    unsigned available                  :1;
    unsigned code_segment_for_64bit     :1;
    unsigned default_operand_size       :1;
    unsigned granularity                :1;

    uint8_t base_high;
};


struct GATE_DISCRIPTOR
{
    uint16_t offset_low;
    uint16_t selector;

    uint8_t unused;

    unsigned gate_type                  :4;
    unsigned storage_segment            :1;
    unsigned descriptor_privilege_level :2;
    unsigned present                    :1;

    uint16_t offset_high;
};

void init_gdtidt(void);
void set_segmdesc(
        struct SEGMENT_DESCRIPTOR *sd,
        uint32_t limit,
        uint32_t base,
        uint8_t accessed,
        uint8_t segment_type,
        uint8_t descriptor_type,
        uint8_t descriptor_privilege_level,
        uint8_t present);


void set_gatedesc(
        struct GATE_DISCRIPTOR *gd,
        uint32_t offset,
        uint32_t selector,
        uint8_t gate_type,
        uint8_t storage_segment,
        uint8_t descriptor_privilege_level,
        uint8_t present);

void init_pic(void);

void init_pit(void);
void set_pit_count(uint16_t count, uint8_t counter, uint8_t mode);

void timer_interrupt(void);

#endif
