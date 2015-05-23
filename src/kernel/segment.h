#ifndef _INCLUDED_SEGMENT_H_
#define _INCLUDED_SEGMENT_H_

#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_IDT 256
#define NUMBER_OF_GDT 8192

typedef struct {
    uint16_t limit_low, base_low;
    uint8_t base_mid;
    unsigned accessed : 1;
    unsigned segment_type : 3;
    unsigned descriptor_type : 1;
    unsigned descriptor_privilege_level : 2;
    unsigned present : 1;

    unsigned limit_high : 4;

    unsigned available : 1;
    unsigned code_segment_for_64bit : 1;
    unsigned default_operand_size : 1;
    unsigned granularity : 1;

    uint8_t base_high;
} segment_descriptor;

typedef enum {
    USER_CODE_SEGMENT_BASE_ADDR = 0x00000000,
    USER_DATA_SEGMENT_BASE_ADDR = 0x00000000,
    USER_CODE_SEGMENT_SIZE = 0xc0000000,
    USER_DATA_SEGMENT_SIZE = 0xc0000000,
    KERNEL_CODE_SEGMENT_BASE_ADDR = 0x00000000,
    KERNEL_DATA_SEGMENT_BASE_ADDR = 0x00000000,
    KERNEL_CODE_SEGMENT_SIZE = 0xffffffff,
    KERNEL_DATA_SEGMENT_SIZE = 0xffffffff,
} segment_infomation;

typedef enum {
    KERNEL_CODE_SEGMENT_INDEX = 2,
    KERNEL_DATA_SEGMENT_INDEX = 3,
    USER_CODE_SEGMENT_INDEX = 4,
    USER_DATA_SEGMENT_INDEX = 5,
    TSS_SEGMENT_INDEX = 6
}gdt_segment_index;

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

bool init_gdt(void);
void set_segment_descriptor(segment_descriptor* sd,
                            uint32_t limit,
                            uint32_t base,
                            uint8_t accessed,
                            uint8_t segment_type,
                            uint8_t descriptor_type,
                            uint8_t descriptor_privilege_level,
                            uint8_t present);

// typedef struct _segment_descriptor
// {
//     uint16_t limit_low, base_low;
//     uint8_t base_mid;
//     unsigned accessed                   :1;
//     unsigned segment_type               :3;
//     unsigned descriptor_type            :1;
//     unsigned descriptor_privilege_level :2;
//     unsigned present                    :1;

//     unsigned limit_high                 :4;

//     unsigned available                  :1;
//     unsigned code_segment_for_64bit     :1;
//     unsigned default_operand_size       :1;
//     unsigned granularity                :1;

//     uint8_t base_high;
// }segment_descriptor;

// static segment_descriptor gdt[NUMBER_OF_GDT];

// bool init_gdt(void);
// void set_segmdesc(segment_descriptor* const sd,
//                   uint32_t limit,
//                   uint32_t base,
//                   uint8_t accessed,
//                   uint8_t segment_type,
//                   uint8_t descriptor_type,
//                   uint8_t descriptor_privilege_level,
//                   uint8_t present);

#endif
