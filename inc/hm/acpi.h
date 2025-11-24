#pragma once

#include <acpi.h>

int acpi_init(struct rsdp_v1_t *rsdp);
int acpi_table_parse_mcfg(struct sdt_header_t *hdr);
