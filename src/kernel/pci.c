#include "pci.h"
#include "pci_device.h"

#include "acpi.h"

static int number_of_pci_devices;

bool init_pci(bool is_support_pci)
{
    if (!is_support_pci)
    {
        return false;
    }

    bool result = true;
    for(int i=0; result; ++i)
    {
        uintptr_t base;
        uint16_t seg_num;
        uint8_t start_bus;
        uint8_t end_bus;
        result = get_mcfgt_info(i, &base, &seg_num, &start_bus, &end_bus);

        number_of_pci_devices = i;
        //TODO
    }

    return true;
}
