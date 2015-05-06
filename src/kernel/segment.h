#ifndef _INCLUDED_SEGMENT_H_
#define _INCLUDED_SEGMENT_H_

#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_IDT 256
#define NUMBER_OF_GDT 8192

typedef struct _segment_descriptor
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
}segment_descriptor;

static segment_descriptor gdt[NUMBER_OF_GDT];

bool init_gdt(void);
void set_segmdesc(segment_descriptor* const sd,
                  uint32_t limit,
                  uint32_t base,
                  uint8_t accessed,
                  uint8_t segment_type,
                  uint8_t descriptor_type,
                  uint8_t descriptor_privilege_level,
                  uint8_t present);

enum segment_infomation {
    USER_SEGMENT_BASE_ADDR = 0x00000000,
    KERNEL_SEGMENT_BASE_ADDR = 0x00000000,
    USER_SEGMENT_SIZE = 0xc0000000,
    KERNEL_SEGMENT_SIZE = 0xffffffff,
};

#endif
