#include <init.h>
#include <x86_64.h>

bool init_mp(void)
{
    uint32_t eax, edx;
    cpuid(1, &eax, &edx);

    {
        char buf[32];
        itoa((uint64_t)edx, buf, 16);
        puts("cpuid: 0x");
        puts(buf);
        puts("\n");
    }

    return true;
}

