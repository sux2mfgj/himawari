#include <init.h>
#include <task_call.h>

#include <stdbool.h>

static void send_init_data(void)
{
    struct Message msg = {
        .source = System,
        .dest   = Memory,
        .type   = Send,
        .content = {1, 2},
    };
    uint64_t result = send(&msg);

}

void system_task(void)
{
    // TODO
    // initialize

    send_init_data();
    while (true)
    {
        // receive
    }
}
