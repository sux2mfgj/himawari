#pragma once

#include <hm/pci.h>
#include <msix.h>

int msix_init(struct pcie_device *pdev, struct msix_capability *cap);
int msix_enable(struct pcie_device *pdev);
int msix_set_vector(struct pcie_device *pdev, uint16_t idx, uint64_t msg_addr,
                    uint32_t msg_data);
