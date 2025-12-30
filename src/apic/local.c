#include <hm/apic.h>
#include <hm/print.h>
#include <hm/vm.h>
#include <stdbool.h>

#define REG_OFFSET_ID (0x020)
#define REG_OFFSET_VER (0x030)
#define REG_OFFSET_TPR (0x080)  // Task Priority Register
#define REG_OFFSET_EOI (0x0B0)  // End of Interrupt Register
#define REG_OFFSET_SIVR (0x0f0)
#define REG_OFFSET_ICR (0x300)
#define REG_OFFSET_ICR_CTRL (0x300)
#define REG_ICR_CTRL_MASK_VEC (0xff)

#define REG_ICR_CTRL_OFFSET_DELIV_MODE (8)
#define ICR_CTRL_DEVLI_MODE_FIXED (0b000)
#define ICR_CTRL_DEVLI_MODE_INIT (0b101)
#define ICR_CTRL_DEVLI_MODE_STARTUP (0b110)

#define REG_ICR_CTRL_MASK_DEST_MODE (0b11 << 12)
#define REG_ICR_CTRL_MASK_DELIV_STATUS (0b11 << 14)
#define REG_ICR_CTRL_MASK_LEVEL (0b11 << 16)

#define REG_ICR_CTRL_OFFSET_LEVEL (16)
#define ICR_CTRL_LEVEL_DE_ASSERT (0b0)
#define ICR_CTRL_LEVEL_ASSERT (0b1)

#define REG_ICR_CTRL_OFFSET_TRIG_MODE 18
#define ICR_CTRL_TRIG_MODE_EDGE (0b0)
#define ICR_CTRL_TRIG_MODE_LEVEL (0b1)

#define REG_ICR_CTRL_MASK_TRIG_MODE (0b11 << 18)

#define REG_ICR_CTRL_MASK_DEST_SHORTHAND (0xff << 24)
#define REG_ICR_CTRL_OFFSET_DEST_SHORTHAND (24)
#define ICR_CTRL_DEST_SHORTHAND_NONE (0b00)
#define ICR_CTRL_DEST_SHORTHAND_SELF (0b01)
#define ICR_CTRL_DEST_SHORTHAND_ALL_INC_SELF (0b10)
#define ICR_CTRL_DEST_SHORTHAND_ALL_EXCL_SELF (0b11)

#define REG_OFFSET_ICR_DST (0x310)
#define REG_OFFSET_LVT (0x320)
#define LVT_ONE_SHOT 0b00
#define LVT_PERIOD 0b01
#define LVT_TSC_DEADLINE 0b10

#define REG_OFFSET_INIT_CR (0x380)
#define REG_OFFSET_CUR_CR (0x390)
#define REG_OFFSET_DCR (0x3e0)
#define REG_OFFSET_IRR (0x200)  // Interrupt Request Register base
#define REG_OFFSET_ISR (0x100)  // In-Service Register base
#define REG_OFFSET_ESR (0x280)  // Error Status Register

struct local_apic {
  uint8_t *base;
};

static struct local_apic bsp_local_apic;
static bool initialized = false;

int local_apic_init(uint32_t base, struct local_apic **lapic) {

  int ret;

  if (initialized)
    return -1;

  // Enable APIC via IA32_APIC_BASE MSR (bit 11)
  uint32_t eax, edx;
  asm volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x1B));
  kprintf("APIC_BASE MSR: 0x%x%x\n", edx, eax);
  if (!(eax & (1 << 11))) {
    kprintf("Enabling APIC via MSR\n");
    eax |= (1 << 11); // Set bit 11 to enable APIC
    asm volatile("wrmsr" : : "a"(eax), "d"(edx), "c"(0x1B));
  }

  uint8_t *reg_base = (uint8_t *)(uintptr_t)base;
  ret = vm_map_device_straight((uint64_t)reg_base, 1); // TODO
  if (ret) {
    kprintf("failed to map the local apic register region\n");
    return ret;
  }

  bsp_local_apic = (struct local_apic){
      .base = reg_base,
  };

  // Enable APIC by setting bit 8 of SIVR and set spurious interrupt vector
  volatile uint32_t *sivr = (uint32_t *)(reg_base + REG_OFFSET_SIVR);
  kprintf("SIVR before: 0x%x\n", *sivr);
  *sivr = 0x1ff; // Enable APIC (bit 8) + spurious vector 0xff
  kprintf("SIVR after: 0x%x\n", *sivr);

  // Set TPR to 0 to allow all interrupts
  volatile uint32_t *tpr = (uint32_t *)(reg_base + REG_OFFSET_TPR);
  *tpr = 0;
  kprintf("TPR set to: 0x%x\n", *tpr);

  // Clear ESR
  volatile uint32_t *esr = (uint32_t *)(reg_base + REG_OFFSET_ESR);
  *esr = 0;
  *esr = 0;  // Write twice as per Intel manual
  kprintf("ESR: 0x%x\n", *esr);

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

enum local_apic_ipi_type {
  LOCAL_APIC_IPI_INIT,
  LOCAL_APIC_IPI_STARTUP,
};

int local_apic_icrc_write(struct local_apic *lapic, uint64_t val) {
  // In xAPIC mode, ICR is written as two 32-bit registers
  // ICR High (offset 0x310): bits 32-63 (destination field)
  // ICR Low (offset 0x300): bits 0-31 (vector, delivery mode, etc.)
  volatile uint32_t *icr_high = (uint32_t *)(lapic->base + REG_OFFSET_ICR_DST);
  volatile uint32_t *icr_low = (uint32_t *)(lapic->base + REG_OFFSET_ICR_CTRL);

  uint32_t high = (uint32_t)(val >> 32);
  uint32_t low = (uint32_t)(val & 0xFFFFFFFF);

  // Write ICR High first (destination)
  *icr_high = high;

  // Then write ICR Low (this triggers the IPI)
  *icr_low = low;

  return 0;
}

void local_apic_set_timer(struct local_apic *lapic, uint8_t vector,
                          uint32_t count) {

  if (!lapic)
    lapic = &bsp_local_apic;

  // Set divide configuration register to divide by 1
  volatile uint32_t *dcr = (uint32_t *)(lapic->base + REG_OFFSET_DCR);
  *dcr = 0b1011; // Divide by 1

  volatile uint32_t *lvt_timer = (uint32_t *)(lapic->base + REG_OFFSET_LVT);

  uint32_t val = (uint32_t)vector | (uint32_t)0 << 12 /* deliver status */ |
                 (uint32_t)0 << 16 /* mask */ | LVT_ONE_SHOT << 17;

  *lvt_timer = val;

  volatile uint32_t *initial_count =
      (uint32_t *)(lapic->base + REG_OFFSET_INIT_CR);

  *initial_count = count;
}

void local_apic_eoi(struct local_apic *lapic) {
  if (!lapic)
    lapic = &bsp_local_apic;

  volatile uint32_t *eoi = (uint32_t *)(lapic->base + REG_OFFSET_EOI);
  *eoi = 0;

  // Memory barrier to ensure EOI write completes
  __asm__ volatile("mfence" ::: "memory");
}

uint32_t local_apic_read_isr(struct local_apic *lapic, uint8_t vector) {
  if (!lapic)
    lapic = &bsp_local_apic;

  // ISR is an array of 8 32-bit registers (0x100-0x170)
  // Each register covers 32 vectors
  uint32_t reg_index = vector / 32;
  uint32_t bit_index = vector % 32;

  volatile uint32_t *isr = (uint32_t *)(lapic->base + REG_OFFSET_ISR + (reg_index * 0x10));
  return (*isr >> bit_index) & 1;
}

uint32_t local_apic_read_irr(struct local_apic *lapic, uint8_t vector) {
  if (!lapic)
    lapic = &bsp_local_apic;

  // IRR is an array of 8 32-bit registers (0x200-0x270)
  // Each register covers 32 vectors
  uint32_t reg_index = vector / 32;
  uint32_t bit_index = vector % 32;

  volatile uint32_t *irr = (uint32_t *)(lapic->base + REG_OFFSET_IRR + (reg_index * 0x10));
  return (*irr >> bit_index) & 1;
}

static inline uint64_t create_icrc_val(uint8_t vector, uint8_t deliv_mode,
                                       uint8_t dest_mode, uint8_t deliv_status,

                                       uint8_t level, uint8_t trigger_mode,
                                       uint8_t dest_short_hand,
                                       uint8_t dest_field) {
  return (uint64_t)(dest_field & 0xff) << 56 |
         (uint64_t)(dest_short_hand & 0b11) << 18 |
         (uint64_t)(trigger_mode & 0b1) << 15 | (uint64_t)(level & 0b1) << 14 |
         (uint64_t)(deliv_status & 0b1) << 12 |
         (uint64_t)(dest_mode & 0b1) << 11 |
         (uint64_t)(deliv_mode & 0b111) << 8 | (uint64_t)vector;
}

void delay(int ms);
int local_apic_start_ap(uint64_t ap_id, void *entry) {

  // INIT
  uint64_t val = create_icrc_val(0, ICR_CTRL_DEVLI_MODE_INIT, 0, 0,
                                 ICR_CTRL_LEVEL_ASSERT, ICR_CTRL_TRIG_MODE_EDGE,
                                 ICR_CTRL_DEST_SHORTHAND_NONE, ap_id);
  local_apic_icrc_write(&bsp_local_apic, val);

  delay(100);

  // SIPI
  uint8_t vector = (uintptr_t)entry / 0x1000;
  val = create_icrc_val(vector, ICR_CTRL_DEVLI_MODE_STARTUP, 0, 0,
                        ICR_CTRL_LEVEL_ASSERT, ICR_CTRL_TRIG_MODE_EDGE,
                        ICR_CTRL_DEST_SHORTHAND_NONE, ap_id);
  local_apic_icrc_write(&bsp_local_apic, val);
  delay(10);

  // SIPI (send twice as per Intel manual)
  local_apic_icrc_write(&bsp_local_apic, val);
  delay(10);

  return 0;
}
