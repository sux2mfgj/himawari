#include "utils.h"

void assert(bool is_pass, const char* message)
{
    if(is_pass)
    {
        return;
    }

    //TODO
    while (true)
    {
        __asm__ volatile("hlt");
    };
}

void printk(const char* message)
{

}
