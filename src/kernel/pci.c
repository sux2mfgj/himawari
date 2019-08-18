#include "pci.h"

bool init_pci(bool is_support_pci)
{
    if(!is_support_pci)
    {
        return false;
    }

    //TODO setup and configuration devices

    return true;
}
