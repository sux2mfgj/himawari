#include <init.h>
#include <task_call.h>

#include <stdbool.h>

static void initialize(void)
{
    uint64_t result;
    struct Message msg = {
        .source = System,
        .dest   = Memory,
        .type   = Receive,
        .content = {
            .address = 0, 
            .num = 0
        },
    };

    // setup memory server
    result = receive(&msg);
    uintptr_t bitmap_addr = msg.content.address;

    msg.dest = Memory;
    msg.type = Send;
    result = send(&msg);

    // setup process
    
}

void system_task(void)
{
    // TODO
    // initialize

    initialize();
    while (true)
    {
        // receive
    }
}
