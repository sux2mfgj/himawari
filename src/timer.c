#include "timer.h"
#include "task.h"


static uint32_t timer_tick = 0;

void timer_interrupt()
{
    timer_tick++;
    scheduler_tick();
    return;
}
