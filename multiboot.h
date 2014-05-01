#ifndef _INCLUDED_MUTIBOOT_H_
#define _INCLUDED_MUTIBOOT_H_

typedef struct {
//     version number
    unsigned flags;
//     avvailable memory
    unsigned mem_lower;
    unsigned mem_upper;
    unsigned boot_device;
    unsigned cmdline;
//     boot module list
    unsigned mods_count;
    unsigned mods_addr;

    unsigned syms1;
    unsigned syms2;
    unsigned syms3;
//     memory mapping buffer
    unsigned mmap_length;
    unsigned mmap_addr;
//     drive info buffer
    unsigned drives_length;
    unsigned drives_addr;
//     ROM configuration table
    unsigned config_table;
//     bootloader name
    unsigned boot_loader_name;
//     video 
    unsigned vbe_control_info;
    unsigned vbe_mode_info;
    unsigned vbe_mode;
    unsigned vbe_interface_seg;
    unsigned vbe_interface_off;
    unsigned vbe_interface_len;

} MULTIBOOT_INFO;

#endif
