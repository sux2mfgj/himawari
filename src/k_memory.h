#ifndef _INCLUDED_K_MEMORY_H_
#define _INCLUDED_K_MEMORY_H_

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "p_memory.h"
#include "multiboot.h"
#include "v_memory.h"



#define MEMORY_INFO_STATUS_FREE 0x00000000
#define MEMORY_INFO_STATUS_USED 0x00000001
#define MEMORY_INFO_STATUS_NODATA 0x00000002
#define MEMORY_INFO_STATUS_END 0x00000003

#define MEMORY_MANAGEMENT_DATA_SIZE 1024
#define KERNEL_HEAP_END 0x00a00000

typedef struct {
    uintptr_t base_addr;
    uint32_t size;
    uint32_t status;
} memory_info;

typedef struct {
    uint32_t end_point;
    uint32_t nodata_elements_count;
    size_t heap_size;
    size_t free_size;
    memory_info data[MEMORY_MANAGEMENT_DATA_SIZE];
} memory_data;

bool memory_management_init(size_t size, uintptr_t base_addr);
void *memory_allocate(uint32_t size);
void *memory_allocate_4k(uint32_t num);
bool memory_free(void *address);
static void memory_management_array_compaction(void);

size_t memtest(uint32_t start, uint32_t end);
// uint32_t memtest_sub(uint32_t start, uint32_t end);

bool init_memory(MULTIBOOT_INFO *multiboot_info);

extern char _bss_end, _text_start, _kernel_start, _kernel_end;

uint32_t get_size_of_kernel(void);

void alloc_free_test(void);
void print_array_status(void);

#endif

