#ifndef _INCLUDED_KVMEMORY_H_
#define _INCLUDED_KVMEMORY_H_

#include "multiboot.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KVMEMORY_SIZE 0x1000
#define MIN_KVMEMORY_ALLOC_SIZE 0x10


typedef struct _kvmem_struct {
    uint32_t number_of_same_state;
    enum {
        FREE,
        ALLOCATED,
    } flag;

} kvmem_struct;

static kvmem_struct kvmem_list[KVMEMORY_SIZE / MIN_KVMEMORY_ALLOC_SIZE];
static uintptr_t base_addr;


bool init_kvmemory(const multiboot_info* mb_info);
void* kvmalloc(uint32_t size);
#endif
