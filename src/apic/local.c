#include <hm/apic.h>
#include <hm/print.h>
#include <hm/vm.h>
#include <stdbool.h>

#define REG_OFFSET_ID (0x020)
#define REG_OFFSET_VER (0x030)
#define REG_OFFSET_EOI (0x080)
#define REG_OFFSET_SIVR (0x0f0)
#define REG_OFFSET_ICR (0x300)
#define REG_OFFSET_ICR_CTRL (0x300)
#define REG_ICR_CTRL_MASK_VEC (0xff)

#define REG_ICR_CTRL_OFFSET_DELIV_MODE (8)
#define ICR_CTRL_DEVLI_MODE_FIXED (0b000 << REG_ICR_CTRL_OFFSET_DELIV_MODE)
#define ICR_CTRL_DEVLI_MODE_INIT (0b101 << REG_ICR_CTRL_OFFSET_DELIV_MODE)
#define ICR_CTRL_DEVLI_MODE_SIPI (0b110 << REG_ICR_CTRL_OFFSET_DELIV_MODE)

#define REG_ICR_CTRL_MASK_DEST_MODE (0b11 << 12)
#define REG_ICR_CTRL_MASK_DELIV_STATUS (0b11 << 14)
#define REG_ICR_CTRL_MASK_LEVEL (0b11 << 16)
#define REG_ICR_CTRL_MASK_TRIG_MODE (0b11 << 18)
#define REG_ICR_CTRL_MASK_DEST_SHORTHAND (0xff << 24)

#define REG_OFFSET_ICR_DST (0x310)
#define REG_OFFSET_LVT (0x320)
#define REG_OFFSET_INIT_CR (0x380)
#define REG_OFFSET_CUR_CR (0x390)

struct local_apic {
  uint8_t *base;
};

static struct local_apic bsp_local_apic;
static bool initialized = false;

int local_apic_init(uint32_t base, struct local_apic **lapic) {

  int ret;

  if (initialized)
    return -1;

  uint8_t *reg_base = (uint8_t *)(uintptr_t)base;
  ret = vm_map_device_straight((uint64_t)reg_base, 1); // TODO
  if (ret) {
    kprintf("failed to map the local apic register region\n");
    return ret;
  }

  bsp_local_apic = (struct local_apic){
      .base = reg_base,
  };

  *lapic = &bsp_local_apic;

  initialized = true;

  return 0;
}

int local_apic_read_id(struct local_apic *lapic, uint32_t *id) {

  if (!lapic)
    return -1;

  *id = *(uint32_t *)(lapic->base + REG_OFFSET_ID);

  return 0;
}
