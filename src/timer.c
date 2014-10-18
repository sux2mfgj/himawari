#include "timer.h"


static uint32_t timer_tick = 0;

void do_timer()
{
    timer_tick++;
    return;
}
