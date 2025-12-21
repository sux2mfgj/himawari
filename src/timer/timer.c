#include <hm/cpuid.h>
#include <hm/print.h>
#include <stddef.h>

// struct tsc {};

static int tsc_freq;

struct local_apic;
void local_apic_set_timer(struct local_apic *lapic, uint8_t vector,
                          uint32_t count);
void local_apic_eoi(struct local_apic *lapic);
void delay(int ms) {
  // Simple busy-wait delay using TSC
  uint64_t start, end;

  // tsc_freq is in KHz (cycles per millisecond), so cycles = tsc_freq * ms
  uint64_t cycles = (uint64_t)tsc_freq * ms;

  // Read TSC: result is in EDX:EAX
  uint32_t low, high;
  asm volatile("rdtsc" : "=a"(low), "=d"(high));
  start = ((uint64_t)high << 32) | low;
  end = start + cycles;

  uint64_t current;
  do {
    asm volatile("rdtsc" : "=a"(low), "=d"(high));
    current = ((uint64_t)high << 32) | low;
  } while (current < end);
}

int timer_init(void) {

  uint32_t max_leaf;
  cpuid_get_max_leaf(&max_leaf);
  kprintf("cpuid: max leaf %d\n", max_leaf);

  uint32_t tsc_numerator;
  uint32_t tsc_denomator;
  uint32_t core_crystal_clock;

  cpuid_get_tsc_info(&tsc_numerator, &tsc_denomator, &core_crystal_clock);

  kprintf("tsc: numerator %d, denometor %d, core crystal clock hz %d\n",
          tsc_numerator, tsc_denomator, core_crystal_clock);

  tsc_freq = core_crystal_clock * tsc_numerator / tsc_denomator;
  kprintf("tsc freq: %d\n", tsc_freq);

  return 0;
}
