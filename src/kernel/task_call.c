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

uint64_t receive(enum ServerType src, struct Message* msg)
{
    msg->type = Receive;      
    msg->src = src;
    return taskcall(msg);
}

uint64_t send(enum ServerType dest, struct Message *msg)
{
    msg->type = Send;
    msg->dest = dest;
    return taskcall(msg);
}
