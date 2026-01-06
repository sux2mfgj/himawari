#include <hm/linker.h>
#include <hm/mm.h>
#include <hm/print.h>
#include <hm/utils.h>
#include <stdbool.h>

bool mm_initialized = false;
bool mm_early_initialized = false;

struct mem_info {
  struct mem_info *next, *prev;
  size_t npages;
};

static struct mem_info free_head;

static void init_free_list(void) {
  free_head = (struct mem_info){
      .next = &free_head,
      .prev = &free_head,
      .npages = 0,
  };
}

static int add_free_list(struct mem_info *entry) {
  struct mem_info *tail_entry = free_head.prev;
  if (tail_entry->next != &free_head) {
    kprintf("mm: free list is invalid state\n");
    return -1;
  }

  tail_entry->next = entry;
  entry->next = &free_head;
  entry->prev = tail_entry;
  free_head.prev = entry;

  return 0;
}

static void remove_free_list(struct mem_info *entry) {
  entry->prev->next = entry->next;
  entry->next->prev = entry->prev;

  entry->next = NULL;
  entry->prev = NULL;
}

static void insert_free_list(struct mem_info *prev, struct mem_info *entry) {
  entry->next = prev->next;
  entry->prev = prev;
  prev->next->prev = entry;
  prev->next = entry;
}

static struct mem_info *find_free_list_entry_containing(uint64_t base,
                                                        size_t npages) {
  struct mem_info *entry = free_head.next;
  while (entry != &free_head) {

    uint64_t entry_base = (uintptr_t)entry;
    uint64_t entry_len = entry->npages * PAGE_SIZE;
    if (entry_base <= base && base < entry_base + entry_len)
      return entry;

    entry = entry->next;
  }

  return NULL;
}

#define PAGE_FLAG_RAM (0x1 << 0)

struct page {
  uint32_t npages;
  uint32_t flags;
};

static struct page *pages = NULL;

static struct page *ptr_to_page(void *ptr) {

  uintptr_t addr = (uintptr_t)ptr;
  int idx_4k = addr >> 12;

  return &pages[idx_4k];
}

static int page_struct_init(uintptr_t highest_ram) {

  if (pages)
    return -1;

  size_t npages = highest_ram / 0x1000;
  pages = mm_alloc(npages * sizeof(struct page));
  if (!pages)
    return -1;

  for (int i = 0; i < npages; i++) {
    pages[i] = (struct page){
        .npages = 1,
    };
  }

  return 0;
}

static void update_page_struct(void *ptr, int npages) {

  struct page *page = ptr_to_page(ptr);

  page->npages = npages;
}

void *mm_alloc(size_t size) {

  size_t npages = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;

  // Search through the free list for an entry large enough
  struct mem_info *entry = free_head.next;
  while (entry != &free_head) {
    if (entry->npages >= npages) {
      void *ptr = entry;

      // Remove the current entry from free list
      remove_free_list(entry);

      // If there are remaining pages, create a new free entry
      if (entry->npages > npages) {
        struct mem_info *next =
            (struct mem_info *)((uint8_t *)entry + npages * PAGE_SIZE);
        next->npages = entry->npages - npages;

        insert_free_list(&free_head, next);
      }

      update_page_struct(ptr, npages);
      return ptr;
    }

    entry = entry->next;
  }

  return NULL;
}

void mm_free(void *ptr) {

  struct mem_info *entry = (struct mem_info *)ptr;
  struct page *page = ptr_to_page(ptr);

  *entry = (struct mem_info){
      .npages = page->npages,
  };

  insert_free_list(&free_head, entry);
}

static int exclude_kernel_space(void) {

  uintptr_t kernel_start_addr = (uintptr_t)&_kernel_start;
  uintptr_t kernel_end_addr = (uintptr_t)&_kernel_end;
  size_t npages = (kernel_end_addr - kernel_start_addr) / PAGE_SIZE;

  kprintf("kernel space 0x%x (%d pages)\n", kernel_start_addr, npages);

  struct mem_info *entry =
      find_free_list_entry_containing(kernel_start_addr, npages);
  if (!entry)
    return -1;

  if (kernel_start_addr == (uintptr_t)entry) {

    struct mem_info *next_entry =
        (struct mem_info *)(kernel_start_addr + npages * PAGE_SIZE);

    next_entry->next = NULL;
    next_entry->prev = NULL;
    next_entry->npages = entry->npages - npages;

    insert_free_list(entry, next_entry);

    // This is a kernel entry
    entry->npages = npages;
  } else {
    return -1;
  }

  return 0;
}

static int exclude_ap_boot_space(void) {

  uintptr_t ap_entry_addr = (uintptr_t)&_ap_start;
  uintptr_t ap_entry_end = (uintptr_t)&_ap_end;

  int n_ap_pages = (ap_entry_end - ap_entry_addr) / PAGE_SIZE;

  struct mem_info *entry =
      find_free_list_entry_containing(ap_entry_addr, n_ap_pages);
  if (!entry)
    return -1;

  if ((uintptr_t)entry == ap_entry_addr)
    return -1;

  int n_whole_pages = entry->npages;
  int n_before_ap_entry_pages = (ap_entry_addr - (uintptr_t)entry) / PAGE_SIZE;
  /*(e)
   * | before ap entry | ap entry code | after ap entry |
   */

  struct mem_info *after_ap_space = (struct mem_info *)ap_entry_end;

  after_ap_space->next = NULL;
  after_ap_space->prev = NULL;
  after_ap_space->npages = n_whole_pages - n_ap_pages - n_before_ap_entry_pages;

  entry->npages = n_before_ap_entry_pages;

  insert_free_list(entry, after_ap_space);

  return 0;
}

static int exclude_null_page(void) {

  struct mem_info *entry = free_head.next;

  if (&free_head == entry)
    return -1;

  // Check if entry starts at address 0
  if ((uintptr_t)entry != 0)
    return 0;

  struct mem_info *new = (struct mem_info *)0x1000;
  new->next = NULL;
  new->prev = NULL;
  new->npages = entry->npages - 1;

  remove_free_list(entry);
  insert_free_list(&free_head, new);

  return 0;
}

static void dump_mmap_table_entry(struct hvm_memmap_table_entry *table) {
  switch (table->type) {
  case HVM_MEMMAP_TYPE_RAM:
    kprintf("RAM: 0x%x(0x%x)\n", table->addr, table->size);
    break;
  case HVM_MEMMAP_TYPE_ACPI:
    kprintf("ACPI: 0x%x\n", table->addr);
    break;
  default:
    kprintf("unknwon(%d): 0x%x\n", table->type, table->addr);
    break;
  }
}

int mm_init(struct hvm_memmap_table_entry *entries, size_t nentry) {
  int ret;

  if (!mm_early_initialized)
    return -1;

  if (mm_initialized)
    return -1;

  // Re-initialize free list for the full memory map
  init_free_list();

  // TODO: free the early heap.

  uintptr_t highest_ram = 0;

  for (int i = 0; i < nentry; i++) {
    struct hvm_memmap_table_entry *entry = &entries[i];

    dump_mmap_table_entry(entry);

    if (entry->type != HVM_MEMMAP_TYPE_RAM)
      continue;

    struct mem_info *free_entry = (struct mem_info *)entry->addr;
    free_entry->next = NULL;
    free_entry->prev = NULL;
    free_entry->npages = entry->size / PAGE_SIZE;

    add_free_list(free_entry);

    highest_ram = max(highest_ram, entry->addr + entry->size);
  }

  ret = exclude_null_page();
  if (ret)
    return ret;

  ret = exclude_kernel_space();
  if (ret)
    return ret;

  ret = exclude_ap_boot_space();
  if (ret)
    return ret;

  ret = page_struct_init(highest_ram);
  if (ret)
    return ret;

  mm_initialized = true;
  return 0;
}

int mm_early_init(void) {

  if (mm_early_initialized)
    return -1;

  uintptr_t early_heap_base = (uintptr_t)&_early_heap_start;
  uintptr_t early_heap_end = (uintptr_t)&_early_heap_end;
  size_t size = early_heap_end - early_heap_base;

  init_free_list();

  struct mem_info *free = (struct mem_info *)early_heap_base;
  free->next = NULL;
  free->prev = NULL;
  free->npages = size / PAGE_SIZE;

  add_free_list(free);

  mm_early_initialized = true;

  return 0;
}
