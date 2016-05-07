#include <x86_64.h>

#include <stdbool.h>
void idle_task(void)
{
    while (true) {
        hlt();
    }
}
