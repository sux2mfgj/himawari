#include "timer.h"


static uint32_t timer_tick = 0;

void timer_interrupt()
{
    timer_tick++;
    return;
}
