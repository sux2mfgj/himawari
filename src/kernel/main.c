#include <stdbool.h>

#include "boot_arg.h"
#include "graphic_text.h"

void start_kernel(struct boot_arg* bootinfo)
{
    bool result;

    result = init_gop_graphic();
    if(!result)
    {
        while(true){}
    }

    print_char('H');

    while (true) {}
}
