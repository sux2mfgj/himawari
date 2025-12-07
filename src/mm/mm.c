#include <hm/linker.h>
#include <hm/mm.h>
#include <hm/print.h>
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

void *mm_alloc(size_t size) {

  struct mem_info *entry = free_head.next;
  if (entry == &free_head) {
    return NULL;
  }

  size_t npages = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;

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

    return ptr;
  }

  return NULL;
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

    *next_entry = (struct mem_info){
        .npages = entry->npages - npages,
    };

    insert_free_list(entry, next_entry);

    // This is a kernel entry
    entry->npages = npages;
  } else {
    return -1;
  }

  return 0;
}

static int exclude_null_page(void) {

  struct mem_info *entry = free_head.next;

  if (&free_head == entry)
    return -1;

  if (entry != NULL)
    return 0;

  struct mem_info *new = (struct mem_info *)0x1000;
  *new = (struct mem_info){
      .npages = entry->npages - 1,
  };
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

static int register_ram_spaces(struct hvm_memmap_table_entry *entries,
                               size_t nentry) {
  for (int i = 0; i < nentry; i++) {
    struct hvm_memmap_table_entry *entry = &entries[i];

    dump_mmap_table_entry(entry);

    if (entry->type != HVM_MEMMAP_TYPE_RAM)
      continue;

    struct mem_info *free_entry = (struct mem_info *)entry->addr;
    *free_entry = (struct mem_info){
        .next = NULL,
        .prev = NULL,
        .npages = entry->size / PAGE_SIZE,
    };

    add_free_list(free_entry);
  }

  return 0;
}

static int remove_null_space(struct hvm_memmap_table_entry *entry,
                             struct mem_info **mem_info) {
  if (entry->addr != (uint64_t)NULL)
    return 0;

  struct mem_info *new = (struct mem_info *)(entry->addr + PAGE_SIZE);

  int npages = entry->size / PAGE_SIZE;

  new->next = NULL;
  new->prev = NULL;
  new->npages = npages - 1; // exclude NULL page.

  *mem_info = new;

  return 1;
}

static int remove_kernel_space(struct hvm_memmap_table_entry *entry,
                               struct mem_info **mem_info) {
  uintptr_t kernel_start_addr = (uintptr_t)&_kernel_start;
  if (entry->addr != kernel_start_addr)
    return 1;

  uintptr_t kernel_end_addr = (uintptr_t)&_kernel_end;
  size_t kernel_size = kernel_end_addr - kernel_start_addr;
  size_t npages = kernel_size / PAGE_SIZE;

  if (entry->size < kernel_size)
    return -1;

  kprintf("kernel space 0x%x (%d pages)\n", kernel_start_addr, npages);

  struct mem_info *after_kernel =
      (struct mem_info *)(kernel_start_addr + npages * PAGE_SIZE);

  after_kernel->next = NULL;
  after_kernel->prev = NULL;
  after_kernel->npages = (entry->size / PAGE_SIZE) - npages;

  *mem_info = after_kernel;

  return 0;
}

int mm_init(struct hvm_memmap_table_entry *entries, size_t nentry) {
  int ret;

  if (!mm_early_initialized)
    return -1;

  if (mm_initialized)
    return -1;

  for (int i = 0; i < nentry; i++) {
    struct hvm_memmap_table_entry *entry = &entries[i];

    dump_mmap_table_entry(entry);

    if (entry->type != HVM_MEMMAP_TYPE_RAM)
      continue;

    struct mem_info *free_entry = (struct mem_info *)entry->addr;
    ret = remove_null_space(entry, &free_entry);
    if (ret)
      goto updated;

    ret = remove_kernel_space(entry, &free_entry);
    if (ret)
      goto updated;

    free_entry->next = NULL;
    free_entry->prev = NULL;
    free_entry->npages = entry->size / PAGE_SIZE;

  updated:
    add_free_list(free_entry);
  }

  // register_ram_spaces(entries, nentry);

  // ret = exclude_ap_boot_space();
  // if (ret < 0)
  //   return ret;

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
