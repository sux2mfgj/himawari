#include <init.h>
#include <descriptor.h>
#include <trap.h>
#include <segment.h>
#include <task_call.h>
#include <kernel.h>
#include <schedule.h>
#include <string.h>
#include <x86_64.h>

#include <stdlib.h>

extern void task_call_handler(void);

bool init_task_call(void)
{
    // TODO
    set_gate_dpl3(IDT_ENTRY_TASK_CALL, &task_call_handler);
    return true;
}

static void send_core(struct Message *msg, struct trap_frame_struct *t_frame)
{
    // TODO
    // check receive task list
    struct task_struct *receving = tl_receving_head;
    if (receving == NULL)
    {
        goto blocking;
    }

    do
    {
        // find wait message for receive
        if (receving->msg_buf.dest = msg->source)
        {
            bool r = active_task(receving);
            if (!r)
            {
                // TODO
                // panic
            }
            // change memory space to receive task
            context_switch(tl_active_head, receving, t_frame);
             
            memcpy(&receving->msg_addr->content, &msg->content, sizeof(struct Content));

            //TODO
            //dequeue from receving list

            return;
        }
    } while (receving->receving_next != tl_receving_head);

blocking:
    task_sending(t_frame, msg);
}

static void receive_core(struct Message *msg, struct trap_frame_struct *t_frame)
{
    struct task_struct *sending = tl_sending_head;
    if (tl_sending_head == NULL)
    {
        goto blocking;
    }

    do
    {
        // sending task
        if (sending->msg_buf.dest == msg->source)
        {
            memcpy(&msg->content, &sending->msg_buf.content,
                   sizeof(struct Content));

            bool r = active_task(sending);
            if (!r)
            {
                // TODO
                // panic(!!);
            }
            //TODO
            //dequeue from sending list

            return;
        }
    } while (sending->sending_next != tl_sending_head);

blocking:
    task_receiving(t_frame, msg);
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
    /*
    if (trap_frame->ret_cs != SERVER_CODE_SEGMENT)
    {
        return;
    }
    */

    struct Message *msg = (struct Message *)trap_frame->rdi;
    switch (msg->dest)
    {
        case Memory:
        case System:
        case Any:
            // case Process:

            break;
        default:
            goto failed;
    }

    switch (msg->type)
    {
        case Send:
            send_core(msg, trap_frame);
            break;

        case Receive:
            receive_core(msg, trap_frame);
            break;

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
