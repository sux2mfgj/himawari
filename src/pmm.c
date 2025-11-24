#include <hm/pmm.h>
#include <hm/print.h>
#include <pvh.h>

#include <stdbool.h>

static bool initialized = false;

#define PAGE_SIZE 0x1000

static int phys_mem_info_size;
static struct mem_block phys_mem_info[128];

#define MEM_FREE_BLOCK_SIZE 128
static struct mem_block mem_free_blocks[MEM_FREE_BLOCK_SIZE];

extern char _kernel_start;
extern char _kernel_end;

static int exclude_kernel_space(void) {

  uintptr_t kernel_start_addr = (uintptr_t)&_kernel_start;
  uintptr_t kernel_end_addr = (uintptr_t)&_kernel_end;
  size_t npages = (kernel_end_addr - kernel_start_addr) / PAGE_SIZE;

  kprintf("kernel space 0x%x (%d pages)\n", kernel_start_addr, npages);

  for (int i = 0; i < phys_mem_info_size; i++)
    mem_free_blocks[i] = phys_mem_info[i];

  bool removed = false;
  for (int i = 0; i < phys_mem_info_size; i++) {
    if (mem_free_blocks[i].base == kernel_start_addr) {
      mem_free_blocks[i].base += npages * PAGE_SIZE;
      mem_free_blocks[i].npages -= npages;

      removed = true;
    }
  }

  if (!removed) {
    kprintf("%s:%d should be fix the pmm initialiation");
    return -1;
  }

  return 0;
}

static void exclude_null_page(void) {
  for (int i = 0; i < MEM_FREE_BLOCK_SIZE; i++) {

    if (mem_free_blocks[i].base)
      continue;

    if (!mem_free_blocks[i].npages)
      break;

    mem_free_blocks[i].npages -= 1;
    mem_free_blocks[i].base = 0x1000;
    break;
  }
}

int pmm_init(struct hvm_memmap_table_entry *table, int nentry) {

  if (initialized)
    return -1;

  for (int i = 0; i < nentry; i++) {
    if (table[i].type != HVM_MEMMAP_TYPE_RAM)
      continue;

    struct mem_block block = {
        .base = table[i].addr,
        .npages = table[i].size / PAGE_SIZE,
    };

    phys_mem_info[phys_mem_info_size] = block;
    mem_free_blocks[phys_mem_info_size] = block;

    kprintf("pmm: ram 0x%x (%d pages)\n",
            phys_mem_info[phys_mem_info_size].base,
            phys_mem_info[phys_mem_info_size].npages);

    phys_mem_info_size++;
  }

  exclude_null_page();

  int ret = exclude_kernel_space();
  if (ret < 0)
    return ret;

  initialized = true;
  return 0;
}

int pmm_get_phys_mem_info(struct mem_block **mem_info, int *nentry) {
  if (!initialized)
    return -1;

  *mem_info = phys_mem_info;
  *nentry = phys_mem_info_size;

  return 0;
}

void *pmm_alloc(size_t size) {

  int npages = size / PAGE_SIZE;

  for (int i = 0; i < MEM_FREE_BLOCK_SIZE; i++) {
    if (mem_free_blocks[i].npages < npages)
      continue;

    mem_free_blocks[i].npages -= npages;
    void *ptr = (void *)mem_free_blocks[i].base;

    mem_free_blocks[i].base += PAGE_SIZE * npages;

    return ptr;
  }

  return NULL;
}
