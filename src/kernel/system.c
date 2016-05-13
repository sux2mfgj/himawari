#include <init.h>
#include <task_call.h>
#include <schedule.h>

#include <stdbool.h>

bool regist_server_list[ServerNum];

static void initialize(void)
{
    for(int i=0; i< ServerNum; ++i)
    {
        regist_server_list[i] = false;
    }
}

void system_task(void)
{
    // TODO
    // initialize

    initialize();
    struct Message msg;
    uint64_t result;
    while (true)
    {
        result = receive(Any, &msg);
        switch (msg.number) {
            case Regist:
                if(msg.content.num >= ServerNum || regist_server_list[msg.content.num])
                {
                    //TODO error
                    panic("this server number is invalid");
                    break;
                }
                regist_server_list[msg.content.num] = true;
                current_task->msg_info.self = msg.content.num;
                break;

            case Initialize:
                //TODO
                break;

            default:
                //TODO send error message(where?)
                break;

        }

        // receive
    }
}
