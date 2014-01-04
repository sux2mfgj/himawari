#ifndef __MULTIBOOT__H
#define __MULTIBOOT__H

// Physical address where kernel is loaded
#define DEF_MM_KERNEL_PHY_BASEADDR    0x00100000

// Magic of Multiboot Header
#define DEF_MBH_MAGIC            0x1BADB002

// Flags of Multiboot Header
#define DEF_MBH_FLAGS_PAGE_ALIGN    0x00000001
#define DEF_MBH_FLAGS_MEMORY_INFO   0x00000002
#define DEF_MBH_FLAGS_VIDEO_MODE    0x00000004
#define DEF_MBH_FLAGS_AOUT_KLUDGE   0x00010000

// Flags of elf format kernel image
#define DEF_MBH_FLAGS           ( DEF_MBH_FLAGS_PAGE_ALIGN | DEF_MBH_FLAGS_MEMORY_INFO )

// Flags of binary format kernel image
#define DEF_MBH_FLAGS_BIN       ( DEF_MBH_FLAGS_PAGE_ALIGN | DEF_MBH_FLAGS_MEMORY_INFO | DEF_MBH_FLAGS_AOUT_KLUDGE )

// Checksum of Multiboot Header
#define DEF_MBH_CHECKSUM        ( - ( DEF_MBH_MAGIC + DEF_MBH_FLAGS     ) )
#define DEF_MBH_CHECKSUM_BIN    ( - ( DEF_MBH_MAGIC + DEF_MBH_FLAGS_BIN ) )

// 
#define DEF_MBH_HEADER_ADDR     DEF_MM_KERNEL_PHY_BASEADDR

// Graphic
#define DEF_MBH_MODE_TYPE       0x00000001
#define DEF_MBH_WIDTH           0x00000050
#define DEF_MBH_HEIGHT          0x00000028
#define DEF_MBH_DEPTH           0x00000000

#endif


#ifndef __MULTIBOOT__S
#ifndef __MULTIBOOT_HEADER_H
#define __MULTIBOOT_HEADER_H

typedef unsigned int uint32;

// Multiboot Header
typedef struct
{
    uint32  magic;
    uint32  flags;
    uint32  checksum;

    uint32  header_addr;
    uint32  load_addr;
    uint32  load_end_addr;
    uint32  bss_end_addr;
    uint32  entry_addr;

    uint32  mode_type;
    uint32  width;
    uint32  height;
    uint32  depth;
}MULTIBOOT_HEADER;

// Multiboot Info
typedef struct
{
    uint32  flags;

    uint32  mem_lower;
    uint32  mem_upper;
    uint32  boot_device;
    uint32  cmdline;

    uint32  mods_count;
    uint32  mods_addr;

    uint32  syms1;
    uint32  syms2;
    uint32  syms3;


    uint32  mmap_length;
    uint32  mmap_addr;

    uint32  drives_length;
    uint32  drives_addr;

    uint32  config_table;

    uint32  boot_loader_name;

    uint32  vbe_control_info;
    uint32  vbe_mode_info;
    uint32  vbe_mode;
    uint32  vbe_interface_seg;
    uint32  vbe_interface_off;
    uint32  vbe_interface_len;
}MULTIBOOT_INFO;

#endif
#endif

