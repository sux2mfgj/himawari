#ifndef _INCLUDED_SEGMENT_H_
#define _INCLUDED_SEGMENT_H_

#include"func.h"

struct SEGMENT_DESCRIPTOR{
    unsigned short limit_low, base_low;
    unsigned char base_mid;
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

    unsigned char base_high;
};

struct GATE_DISCRIPTOR{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

inline void init_gdtidt(void);
static void set_segmdesc(
        struct SEGMENT_DESCRIPTOR *sd,
        unsigned int limit,
        unsigned int base,
        unsigned char accessed,
        unsigned char segment_type,
        unsigned char descriptor_type,
        unsigned char descriptor_privilege_level,
        unsigned char present);


static void set_gatedesc(
        struct GATE_DISCRIPTOR *gd,
        int offset,
        int selector,
        int ar);

inline void init_pic(void);

#define GDT_ADDR 0x00270000
#define IDT_ADDR 0x0026f800

#define NUM_IDT 256
#define NUM_GDT 8192


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
