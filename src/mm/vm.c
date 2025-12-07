#include <hm/mm.h>
#include <hm/print.h>
#include <hm/string.h>
#include <stdbool.h>

#include "mm.h"

static bool vm_initialized = false;

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

static uint64_t pml4[512] __attribute__((aligned(4096)));

enum page_size {
  PAGE_SIZE_4K,
  PAGE_SIZE_2M,
  PAGE_SIZE_1G,
};

static inline int get_pml4_index(uint64_t vaddr) {
  return (vaddr >> 39) & 0x1ff;
}

static inline int get_pdpt_index(uint64_t vaddr) {
  return (vaddr >> 30) & 0x1ff;
}

static inline int get_pd_index(uint64_t vaddr) { return (vaddr >> 21) & 0x1ff; }

static inline int get_pt_index(uint64_t vaddr) { return (vaddr >> 12) & 0x1ff; }

static inline uint64_t *get_next_table(uint64_t entry) {
  return (uint64_t *)(entry & ~0xfffULL); // Clear lower 12 bits (flags)
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

struct mem_block {
  uint64_t base;
  int npages;
};

struct mmap_param {
  int write;
  int user;
  int write_through;
  int cache_disable;
};

static int set_table_entry(uint64_t *entry, enum page_size psize, uint64_t addr,
                           struct mmap_param *param) {

  if (*entry & PTE_PRESENT) {
    kprintf("already mapped range\n");
    return -1;
  }

  uint64_t flags = PTE_PRESENT;
  if (param->write)
    flags |= PTE_WRITE;
  if (param->user)
    flags |= PTE_USER;
  if (param->write_through)
    flags |= PTE_MASK_CACHE_WT;
  if (param->cache_disable)
    flags |= PTE_MASK_CACHE_DISABLE;

  uint64_t mask;
  switch (psize) {
  case PAGE_SIZE_1G:
    kprintf("not supported\n");
    return -1;
  case PAGE_SIZE_2M:
    mask = ~0x1fffffULL;         // 2MB = 21 bits
    flags |= PTE_MASK_PAGE_SIZE; // Set PS bit for 2MB pages
    break;
  case PAGE_SIZE_4K:
    mask = ~0xfffULL; // 4KB = 12 bits
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
    pdpt = mm_alloc(0x1000);
    memset(pdpt, 0x00, 0x1000);

    set_table_entry_for_next_table(pml4_entry, pdpt, 1, 0, 0, 0);
  }

  int pdpt_idx = get_pdpt_index(vaddr);
  uint64_t *pdpt_entry = &pdpt[pdpt_idx];
  uint64_t *pd = get_next_table(*pdpt_entry);

  if (!(*pdpt_entry & PTE_MASK_PRESENT)) {
    pd = mm_alloc(0x1000);
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
    pt = mm_alloc(0x1000);
    memset(pt, 0x00, 0x1000);

    set_table_entry_for_next_table(pd_entry, pt, 1, 0, 0, 0);
  }

  int pt_idx = get_pt_index(vaddr);
  uint64_t *pt_entry = &pt[pt_idx];

  return pt_entry;
}

static int vmm_map_straight_2m(struct mem_block *block,
                               struct mmap_param *param) {

  uint64_t *entry = page_walk(block->base, PAGE_SIZE_2M);
  if (!entry) {
    kprintf("Failed to setup the page table entry for 2m\n");
    return -1;
  }

  return set_table_entry(entry, PAGE_SIZE_2M, block->base, param);
}

static int vmm_map_straight_4k(struct mem_block *block,
                               struct mmap_param *param) {
  uint64_t *entry = page_walk(block->base, PAGE_SIZE_4K);
  if (!entry) {
    kprintf("Failed to setup the page table entry for 4k\n");
    return -1;
  }
  return set_table_entry(entry, PAGE_SIZE_4K, block->base, param);
}

int _vmm_map_straight(struct mem_block *block, struct mmap_param *param) {
  int ret;

  // check if 2M aligned
  if (!(block->base % 0x200000) && block->npages >= 512) {
    struct mem_block block_2m = {
        .base = block->base,
        .npages = 512,
    };

    ret = vmm_map_straight_2m(&block_2m, param);
    if (ret < 0)
      return ret;

    block->base += 0x200000;
    block->npages -= 512;

    return 0;
  }

  uint64_t base = block->base;
  // check if 4K aligned
  if (!(base % PAGE_SIZE))
    base = base & ~(PAGE_SIZE - 1);

  struct mem_block block_4k = {
      .base = block->base,
      .npages = 1,
  };
  ret = vmm_map_straight_4k(&block_4k, param);
  if (ret < 0)
    return ret;

  block->base += PAGE_SIZE;
  block->npages -= 1;

  return 0;
}

int vmm_map_straight(struct mem_block *block, struct mmap_param *param) {
  while (block->npages) {
    int ret = _vmm_map_straight(block, param);
    if (ret < 0)
      return ret;
  }

  return 0;
}

int vm_map_ram_straight(uint64_t base, size_t npages) {
  // int vmm_map_ram_straight(struct mem_block *block) {

  struct mem_block block = {
      .base = base,
      .npages = npages,
  };

  struct mmap_param param = {
      .write = 1,
  };

  return vmm_map_straight(&block, &param);
}

int vm_map_device_straight(uint64_t base, size_t npages) {
  struct mem_block block = {
      .base = base,
      .npages = npages,
  };
  // int vm_map_device(struct mem_block *block) {
  struct mmap_param param = {
      .write = 1,
      .write_through = 1,
      .cache_disable = 1,
  };

  return vmm_map_straight(&block, &param);
}

static void set_cr3(uint64_t pml4) {
  asm volatile("mov %0, %%cr3" : : "r"(pml4) : "memory");
}

int vm_init(struct hvm_memmap_table_entry *entries, size_t nentry) {

  // Should be initialized mm firstly.
  if (!mm_early_initialized)
    return -1;

  if (vm_initialized)
    return -1;

  memset(pml4, 0, sizeof(pml4));

  int ret;

  for (int i = 0; i < nentry; i++) {
    struct hvm_memmap_table_entry *entry = &entries[i];

    // Skip entries with invalid size
    // - size must be > 0
    // - size >= 4GB is suspicious for RESERVED/ACPI entries (likely corrupt
    // data)
    if (entry->size == 0 || entry->size >= (1ULL << 32))
      continue;

    // Map RAM, RESERVED, and ACPI regions
    if (entry->type != HVM_MEMMAP_TYPE_RAM &&
        entry->type != HVM_MEMMAP_TYPE_RESERVED &&
        entry->type != HVM_MEMMAP_TYPE_ACPI)
      continue;

    int npages = entry->size / PAGE_SIZE;

    // Use different mapping functions based on type
    if (entry->type == HVM_MEMMAP_TYPE_RAM) {
      // RAM: cached, write-back
      ret = vm_map_ram_straight(entry->addr, npages);
    } else {
      // RESERVED/ACPI: uncached (write-through + cache-disable)
      ret = vm_map_device_straight(entry->addr, npages);
    }

    if (ret < 0) {
      kprintf("Failed to map the range: 0x%x(%d npages)\n", entry->addr,
              npages);
      return -1;
    }
  }

  set_cr3((uint64_t)pml4);

  vm_initialized = true;

  return 0;
}
