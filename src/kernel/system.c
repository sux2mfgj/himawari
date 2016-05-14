#include <init.h>
#include <task_call.h>
#include <schedule.h>
#include <system.h>

#include <stdbool.h>

bool regist_server_list[ServerNum];

static void initialize(void)
{
    for(int i=0; i< ServerNum; ++i)
    {
        regist_server_list[i] = false;
    }
}

bool (*init_task_table[ServerNum])(struct Message* msg) = {
    memory_server_init,
};

void system_task(void)
{
    
    initialize();
    bool res_bool = true; 
    struct Message msg;
    uint64_t result;
    while (true)
    {
        result = receive(Any, &msg);

        switch(msg.src)
        {
            case Memory:
            case System:
                break;
            default:
                //TODO 
                // send error message
                continue;
        }

        switch(msg.number)
        {
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
                //res_bool = (*init_task_table[msg.src])(&msg);
                if(!res_bool)
                {
                    
                }
                break;
            default:
                //TODO send error message
                break;
        }

    }
}
