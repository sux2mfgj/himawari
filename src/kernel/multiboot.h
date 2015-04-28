#ifndef _INCLUDED_MUTIBOOT_H_
#define _INCLUDED_MUTIBOOT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
//     version number
    uint32_t flags;
//     avvailable memory
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
//     boot module list
    uint32_t mods_count;
    uint32_t mods_addr;

    uint32_t syms1;
    uint32_t syms2;
    uint32_t syms3;
//     memory mapping buffer
    uint32_t mmap_length;
    uint32_t mmap_addr;
//     drive info buffer
    uint32_t drives_length;
    uint32_t drives_addr;
//     ROM configuration table
    uint32_t config_table;
//     bootloader name
    uint32_t boot_loader_name;
//     video
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;

} MULTIBOOT_INFO;

#endif
