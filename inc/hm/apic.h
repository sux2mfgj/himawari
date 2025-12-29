#pragma once

#include <stdint.h>

struct local_apic;

int local_apic_init(uint32_t base, struct local_apic **lapic);
int local_apic_read_id(struct local_apic *lapic, uint32_t *id);
void local_apic_eoi(struct local_apic *lapic);
int local_apic_start_ap(uint64_t ap_id, void *entry);
