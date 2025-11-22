#include <stdint.h>

#define HVM_START_MAGIC_VALUE 0x336ec578

struct hvm_start_info {
  uint32_t magic;         /* 'xEn3' with the 0x80 bit of the "E" set. */
  uint32_t version;       /* Version of this structure. */
  uint32_t flags;         /* SIF_xxx flags. */
  uint32_t nr_modules;    /* Number of modules passed to the kernel. */
  uint64_t modlist_paddr; /* Physical address of an array of modules. */
  uint64_t cmdline_paddr; /* Physical address of the command line. */
  uint64_t rsdp_paddr; /* Physical address of the RSDP ACPI data structure. */

  /* メモリマップ情報 */
  uint64_t memmap_paddr;   /* Memory mapの物理アドレス */
  uint32_t memmap_entries; /* Memory mapのエントリ数 */
  uint32_t memmap_version; /* Memory mapのフォーマットバージョン */

  uint32_t reserved; /* Reserved. */
};

struct hvm_memmap_table_entry {
  uint64_t addr; /* Base address */
  uint64_t size; /* Size */
  uint32_t type; /* Type (RAM, Reserved, ACPI, etc.) */
  uint32_t reserved;
};
