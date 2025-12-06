#include <hm/apic.h>
#include <hm/print.h>
#include <hm/vmm.h>

#define LOCAL_APIC_REG_OFFSET_ID (0x20)
#define LOCAL_APIC_REG_OFFSET_VER (0x30)
// #define LOCAL_APIC_REG_OFFSET_
//...

struct local_apic {
  uint8_t *base;
};

static struct local_apic bsp_local_apic;

int local_apic_init(uint32_t base, struct local_apic **lapic) {

  int ret;

  uint8_t *reg_base = (uint8_t *)(uintptr_t)base;

  struct mem_block block = {
      .base = (uint64_t)reg_base,
      .npages = 1,
  };
  ret = vmm_map_device(&block);
  if (ret) {
    kprintf("failed to map the local apic register region\n");
    return ret;
  }

  bsp_local_apic = (struct local_apic){
      .base = reg_base,
  };

  *lapic = &bsp_local_apic;

  return 0;
}

int local_apic_read_id(struct local_apic *lapic, uint32_t *id) {

  if (!lapic)
    return -1;

  *id = *(uint32_t *)(lapic->base + LOCAL_APIC_REG_OFFSET_ID);

  return 0;
}
