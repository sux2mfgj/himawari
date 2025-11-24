#include <hm/pmm.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/vmm.h>
#include <stdbool.h>
#include <stdint.h>

#define PTE_MASK_PRESENT (1 << 0)
#define PTE_PRESENT 1

#define PTE_MASK_RW (1 << 1)
#define PTE_WRITE (1 << 1)
#define PTE_READ (0 << 1)

#define PTE_MASK_USER_SUPER (1 << 2)
#define PTE_USER (1 << 2)
#define PTE_KERNEL (0 << 2)

#define PTE_MASK_CACHE_WT (1 << 3)
// TODO
#define PTE_MASK_CACHE_DISABLE (1 << 4)
// TODO

#define PTE_MASK_ACCESS (1 << 5)

#define PTE_MASK_DIRTY (1 << 6)

#define PTE_MASK_PAGE_SIZE (1 << 7)

static uint64_t pml4[512];

enum page_size {
  PAGE_SIZE_4K,
  PAGE_SIZE_2M,
  PAGE_SIZE_1G,
};

static bool initialized = false;

static inline int get_pml4_index(uint64_t vaddr) {
  return (vaddr >> 39) & 0x1ff;
}

static inline int get_pdpt_index(uint64_t vaddr) {
  return (vaddr >> 30) & 0x1ff;
}

static inline int get_pd_index(uint64_t vaddr) { return (vaddr >> 21) & 0x1ff; }

static inline int get_pt_index(uint64_t vaddr) { return (vaddr >> 12) & 0x1ff; }

static inline uint64_t *get_next_table(uint64_t entry) {
  return (uint64_t *)(entry & ~0xfffULL);  // Clear lower 12 bits (flags)
}

static void set_table_entry_for_next_table(uint64_t *entry, uint64_t *table,
                                           int write, int user,
                                           int write_through,
                                           int cache_disable) {

  uint64_t flags = PTE_PRESENT;
  if (write)
    flags |= PTE_WRITE;
  if (user)
    flags |= PTE_USER;
  if (write_through)
    flags |= PTE_MASK_CACHE_WT;
  if (cache_disable)
    flags |= PTE_MASK_CACHE_DISABLE;

  *entry = (uint64_t)table | flags;
}

static int set_table_entry(uint64_t *entry, enum page_size psize, uint64_t addr,
                           int write, int user, int write_through,
                           int cache_disable) {

  if (*entry & PTE_PRESENT) {
    kprintf("already mapped range\n");
    return -1;
  }

  uint64_t flags = PTE_PRESENT;
  if (write)
    flags |= PTE_WRITE;
  if (user)
    flags |= PTE_USER;
  if (write_through)
    flags |= PTE_MASK_CACHE_WT;
  if (cache_disable)
    flags |= PTE_MASK_CACHE_DISABLE;

  uint64_t mask;
  switch (psize) {
  case PAGE_SIZE_1G:
    kprintf("not supported\n");
    return -1;
  case PAGE_SIZE_2M:
    mask = ~0x1fffffULL;  // 2MB = 21 bits
    flags |= PTE_MASK_PAGE_SIZE;  // Set PS bit for 2MB pages
    break;
  case PAGE_SIZE_4K:
    mask = ~0xfffULL;  // 4KB = 12 bits
    // No PS bit for 4KB pages
    break;
  }

  *entry = (addr & mask) | flags;

  return 0;
}

static uint64_t *page_walk(uint64_t vaddr, enum page_size psize) {

  int pml4_idx = get_pml4_index(vaddr);
  uint64_t *pml4_entry = &pml4[pml4_idx];
  uint64_t *pdpt = get_next_table(*pml4_entry);

  if (!(*pml4_entry & PTE_MASK_PRESENT)) {
    pdpt = pmm_alloc(0x1000);
    memset(pdpt, 0x00, 0x1000);

    set_table_entry_for_next_table(pml4_entry, pdpt, 1, 0, 0, 0);
  }

  int pdpt_idx = get_pdpt_index(vaddr);
  uint64_t *pdpt_entry = &pdpt[pdpt_idx];
  uint64_t *pd = get_next_table(*pdpt_entry);

  if (!(*pdpt_entry & PTE_MASK_PRESENT)) {
    pd = pmm_alloc(0x1000);
    memset(pd, 0x00, 0x1000);

    set_table_entry_for_next_table(pdpt_entry, pd, 1, 0, 0, 0);
  }

  int pd_idx = get_pd_index(vaddr);
  uint64_t *pd_entry = &pd[pd_idx];

  if (psize == PAGE_SIZE_2M) {
    return pd_entry;
  }

  // For 4KB pages, we need PT (Page Table)
  uint64_t *pt = get_next_table(*pd_entry);
  if (!(*pd_entry & PTE_MASK_PRESENT)) {
    pt = pmm_alloc(0x1000);
    memset(pt, 0x00, 0x1000);

    set_table_entry_for_next_table(pd_entry, pt, 1, 0, 0, 0);
  }

  int pt_idx = get_pt_index(vaddr);
  uint64_t *pt_entry = &pt[pt_idx];

  return pt_entry;
}

static int vmm_map_ram_straight_2m(struct mem_block *block) {

  uint64_t *entry = page_walk(block->base, PAGE_SIZE_2M);
  if (!entry) {
    kprintf("Failed to setup the page table entry for 2m\n");
    return -1;
  }

  return set_table_entry(entry, PAGE_SIZE_2M, block->base, 1, 0, 0, 0);
}

static int vmm_map_ram_straight_4k(struct mem_block *block) {
  uint64_t *entry = page_walk(block->base, PAGE_SIZE_4K);
  if (!entry) {
    kprintf("Failed to setup the page table entry for 4k\n");
    return -1;
  }
  return set_table_entry(entry, PAGE_SIZE_4K, block->base, 1, 0, 0, 0);
}

int vmm_map_ram_straight(struct mem_block *block) {
  int ret;

  // check if 2M aligned
  if (!(block->base % 0x200000) && block->npages >= 512) {
    struct mem_block block_2m = {
        .base = block->base,
        .npages = 512,
    };

    ret = vmm_map_ram_straight_2m(&block_2m);
    if (ret < 0)
      return ret;

    block->base += 0x200000;
    block->npages -= 512;

    return 0;
  }

  // check if 4K aligned
  if (!(block->base % 0x1000)) {
    struct mem_block block_4k = {
        .base = block->base,
        .npages = 1,
    };
    ret = vmm_map_ram_straight_4k(&block_4k);
    if (ret < 0)
      return ret;

    block->base += 0x1000;
    block->npages -= 1;

    return 0;
  }

  return -1;
}

static void set_cr3(uint64_t pml4) {
  asm volatile("mov %0, %%cr3" : : "r"(pml4) : "memory");
}

/*
 * This function should be called after `pmm_init()`.
 */
int vmm_init(void) {

  if (initialized)
    return -1;

  int ret;
  struct mem_block *phys_mem_blocks;
  int n_phys_mem_block;
  ret = pmm_get_phys_mem_info(&phys_mem_blocks, &n_phys_mem_block);
  if (ret < 0)
    return -1;

  for (int i = 0; i < n_phys_mem_block; i++) {
    struct mem_block block = phys_mem_blocks[i];
    while (block.npages) {
      ret = vmm_map_ram_straight(&block);
      if (ret < 0) {
        kprintf("Failed to map the ram range: 0x%x(%d npages)\n", block.base,
                block.npages);
        return -1;
      }
    }
  }

  set_cr3((uint64_t)pml4);

  return 0;
}
