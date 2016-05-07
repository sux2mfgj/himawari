#include <task.h>

static uint64_t taskcall(struct Message* msg)
{

    uint64_t result;
    __asm__ volatile(
            "int $0x81;"
            :"=a"(result)
            :"D"(msg));

    return result;
}

uint64_t receive(struct Message* msg)
{
    msg->type = Receive;      
    return taskcall(msg);
}

uint64_t send(struct Message *msg)
{
    msg->type = Send;
    return taskcall(msg);
}
