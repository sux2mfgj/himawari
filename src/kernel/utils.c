#include "utils.h"
#include <stdarg.h>

void assert(bool is_pass, const char *message)
{
    if (is_pass)
    {
        return;
    }

    // TODO
    while (true)
    {
        __asm__ volatile("hlt");
    };
}

void printk(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    // TODO
    va_end(args);
}
