#ifndef _INCLUDED_SEGMENT_H_
#define _INCLUDED_SEGMENT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "task.h"

// #include"func.h"

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

// for tss
#define SEG_TYPE_TSS_16_AVAIL 0x1
#define SEG_TYPE_TSS_16_BUSY 0x3
#define SEG_TYPE_TSS_32_BUSY 0x9
#define SEG_TYPE_TSS_32_AVAIL 0xb

#define DESC_TYPE_SYSTEM    0
#define DESC_TYPE_SEGMENT   1

#define PRESENT     1
#define NOT_PRESENT 0

enum segment_descriptor_variable {
    KERNEL_CODE_SEGMENT = 2,
    KERNEL_DATA_SEGMENT = 3,
    USER_CODE_SEGMENT = 4,
    USER_DATA_SEGMENT = 5,
    TSS_SEGMENT = 6
};
// #define KERNEL_CODE_SEGMENT_NUM 2
// #define DATA_SEGMENT_NUM 3

#define GATE_TYPE_32BIT_TASK    0x5
#define GATE_TYPE_16BIT_INT     0x6
#define GATE_TYPE_16BIT_TRAP    0x7
#define GATE_TYPE_32BIT_INT     0xe
#define GATE_TYPE_32BIT_TRAP    0xf

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

void init_gdt(void);
void set_segmdesc(
        struct SEGMENT_DESCRIPTOR *sd,
        uint32_t limit,
        uint32_t base,
        uint8_t accessed,
        uint8_t segment_type,
        uint8_t descriptor_type,
        uint8_t descriptor_privilege_level,
        uint8_t present);

void init_pic(void);

void init_pit(void);
void set_pit_count(uint16_t count, uint8_t counter, uint8_t mode);

void init_tss(void);
void load_sp0(uintptr_t esp);


void timer_interrupt(void);

#endif
