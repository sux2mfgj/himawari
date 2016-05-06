#include <init.h>
#include <descriptor.h>
#include <trap.h>
#include <segment.h>
#include <task_call.h>
#include <kernel.h>

extern void task_call_handler(void);

bool init_task_call(void)
{
    // TODO
    set_gate_dpl3(IDT_ENTRY_TASK_CALL, &task_call_handler);
    return true;
}

// task call arguments
//
// rax : task call number(when task_call is 0x81)
// rdi : first argument
//  - struct message *  (message)
//
// ---- reserved ---
// rsi : second argument
// rdx : third argument
//
void task(struct trap_frame_struct *trap_frame)
{
    if (trap_frame->ret_cs != SERVER_CODE_SEGMENT)
    {
        return;
    }

    struct Message *msg = (struct Message *)trap_frame->rdi;

    switch (msg->type)
    {
        case Send:
        case Receive:
            break;
        default:
            goto failed;
    }

    switch (msg->dest)
    {
        //        case Memory:
        //        case Process:
        //           break;
        default:
            goto failed;
    }

    trap_frame->rax = 0;
    return;
failed:
    trap_frame->rax = -1;
}

bool init_syscall(void)
{
    bool state = true;
    state = init_task_call();
    if (!state)
    {
        return false;
    }

    // TODO setup system call

    return true;
}
