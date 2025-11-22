#include <hm/pmm.h>
#include <hm/print.h>
#include <pvh.h>

#include <stdbool.h>

static bool initialized = false;

#define PAGE_SIZE 0x1000

struct mem_block {
  uint64_t base;
  int npages;
};

static int mem_info_size;
static struct mem_block mem_info[128];

extern char _kernel_start;
extern char _kernel_end;

static void exclude_kernel_space(void) {

  uintptr_t kernel_start_addr = (uintptr_t)&_kernel_start;
  uintptr_t kernel_end_addr = (uintptr_t)&_kernel_end;
  size_t npages = (kernel_end_addr - kernel_start_addr) / PAGE_SIZE;

  kprintf("kernel space 0x%x (%d pages)\n", kernel_start_addr, npages);
  // for (int i = 0; i < mem_info_size; i++) { }
  // TODO:
}

int pmm_init(struct hvm_memmap_table_entry *table, int nentry) {

  if (initialized)
    return -1;

  for (int i = 0; i < nentry; i++) {
    if (table[i].type != HVM_MEMMAP_TYPE_RAM)
      continue;

    mem_info[mem_info_size] = (struct mem_block){
        .base = table[i].addr,
        .npages = table[i].size / PAGE_SIZE,
    };

    kprintf("pmm: ram 0x%x (%d pages)\n", mem_info[mem_info_size].base,
            mem_info[mem_info_size].npages);

    mem_info_size++;
  }

  exclude_kernel_space();

  initialized = true;
  return 0;
}

void *pmm_alloc(size_t size) {

  int npages = size / PAGE_SIZE;

  for (int i = 0; i < mem_info_size; i++) {
    if (mem_info[i].npages < npages)
      continue;

    mem_info[i].npages -= npages;
    void *ptr = (void *)mem_info[i].base;

    mem_info[i].base += PAGE_SIZE * npages;

    return ptr;
  }

  return NULL;
}
