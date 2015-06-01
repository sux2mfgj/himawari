#include "segment.h"
#include "graphic.h"
#include "paging.h"
#include "trap.h"
#include "kernel.h"
#include "asm.h"

/* static segment_descriptor gdt[NUMBER_OF_GDT]; */

bool init_gdt(void)
{

    gdt = (segment_descriptor* )GDT_ADDR;
    printk("gdt: 0x%x", gdt);

    for (int i = 0; i < NUMBER_OF_GDT; ++i) {
        set_segment_descriptor(gdt + i, 0, 0, 0, 0, 0, 0, 0);
    }

    set_segment_descriptor(gdt + KERNEL_CODE_SEGMENT_INDEX,
                           KERNEL_CODE_SEGMENT_SIZE,
                           KERNEL_CODE_SEGMENT_BASE_ADDR,
                           0,
                           SEG_TYPE_CODE_XRC,
                           DESC_TYPE_SEGMENT,
                           PRIVILEGE_OS,


                           PRESENT);

    set_segment_descriptor(gdt + KERNEL_DATA_SEGMENT_INDEX,
                           KERNEL_DATA_SEGMENT_SIZE,
                           KERNEL_DATA_SEGMENT_BASE_ADDR,
                           0,
                           SEG_TYPE_DATE_REW,
                           DESC_TYPE_SEGMENT,
                           PRIVILEGE_OS,
                           PRESENT);

    set_segment_descriptor(gdt + USER_CODE_SEGMENT_INDEX,
                           USER_CODE_SEGMENT_SIZE,
                           USER_CODE_SEGMENT_BASE_ADDR,
                           0,
                           SEG_TYPE_DATE_REW,
                           DESC_TYPE_SEGMENT,
                           PRIVILEGE_USER,
                           PRESENT);

    set_segment_descriptor(gdt + USER_DATA_SEGMENT_INDEX,
                           USER_DATA_SEGMENT_SIZE,
                           USER_DATA_SEGMENT_BASE_ADDR,
                           0,
                           SEG_TYPE_DATE_REW,
                           DESC_TYPE_SEGMENT,
                           PRIVILEGE_USER,
                           PRESENT);

    // TODO: have to set task state segment
    load_gdtr(sizeof(segment_descriptor) * (NUMBER_OF_GDT - 1),
              (uintptr_t)gdt);

    return true;
}

void set_segment_descriptor(segment_descriptor* sd,
                            uint32_t limit,
                            uint32_t base,
                            uint8_t accessed,
                            uint8_t segment_type,
                            uint8_t descriptor_type,
                            uint8_t descriptor_privilege_level,
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
