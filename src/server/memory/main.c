#include <task_call.h>
#include <process.h>
#include <kernel.h>

int send_message(void)
{
    struct message msg;
    int64_t ret_value;
    msg.source = MEMORY;
    msg.destination = SCHEDULE;

    msg.type = SEND; 

    __asm__ volatile(
            "int $0x81;"
            :"=a"(ret_value)
            :"D"(&msg));
/*     __asm__ volatile( */
/*             "movq %1, %%rdi;" */
/*             "int $0x81;" */
/*             "movq %%rax, %0;" */
/*             :"=r"(ret_value) */
/*             :"r"(&msg) */
/*             : "%rax", "%rdi"); */

    return ret_value;
}

void main(void)
{
    
    int return_value = send_message();
    
    while(1){}
}
