#include <descriptor.h>
#include <init.h>
#include <kernel.h>
#include <schedule.h>
#include <segment.h>
#include <string.h>
#include <task_call.h>
#include <trap.h>
#include <x86_64.h>

#include <schedule.h>
#include <stdlib.h>

extern void task_call_handler(void);

bool init_task_call(void)
{
    // TODO
    set_gate_dpl3(IDT_ENTRY_TASK_CALL, &task_call_handler);
    return true;
}

static void mp_core(enum MessageType type, struct task_struct *t,
                    struct Message *msg, struct trap_frame_struct *t_frame)
{
    bool is_head = false;
    switch (type)
    {
        case Send:
        {
            context_switch(current_task, t, t_frame);

            memcpy(&t->msg_info.addr->content, &msg->content,
                   sizeof(struct Content));
            memcpy(&t->msg_info.addr->src, &current_task->msg_info.self,
                   sizeof(enum ServerType));
            memcpy(&t->msg_info.addr->number, &msg->number,
                   sizeof(enum TaskCallType));

            is_head = true;
            break;
        }
        case Receive:
        {
            memcpy(&msg->content, &t->msg_info.buf.content,
                   sizeof(struct Content));
            memcpy(&msg->src, &t->msg_info.self, sizeof(enum ServerType));

            memcpy(&msg->number, &t->msg_info.buf.number,
                   sizeof(enum TaskCallType));
            break;
        }

        default:
            while (true)
            {
            }
    }

    bool r = active_task(t, is_head);
    if (!r)
    {
    }
}

static void message_passing(struct Message *msg,
                            struct trap_frame_struct *t_frame)
{
    if (suspend_head == NULL)
    {
        goto blocking;
    }

    struct task_struct *c_task = current_task;

    struct linked_list *s_task = suspend_head;
    do
    {
        struct task_struct *t =
            container_of(s_task, struct task_struct, suspend_list);

        switch (msg->type)
        {
            case Send:
            {

                if (t->msg_info.buf.type != Receive)
                {
                    continue;
                }

                // check receive msg
                if (t->msg_info.buf.src != Any &&
                    t->msg_info.buf.src != c_task->msg_info.self)
                {
                    continue;
                }

                // check send msg
                if (msg->dest != t->msg_info.self)
                {
                    continue;
                }

                mp_core(Send, t, msg, t_frame);
                return;
            }
            case Receive:
            {
                if (t->msg_info.buf.type != Send)
                {
                    continue;
                }

                // check send msg
                if (t->msg_info.buf.dest != c_task->msg_info.self)
                {
                    continue;
                }

                // check receive msg
                if (msg->dest != Any && msg->dest != t->msg_info.self)
                {
                    continue;
                }

                mp_core(Receive, t, msg, t_frame);
                return;
            }
            default:
                // TODO error
                while (true)
                {
                }
        }

    } while (s_task != suspend_head);

blocking:
    suspend_task(t_frame, msg);
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
    switch (trap_frame->ret_cs)
    {
        case SERVER_CODE_SEGMENT:
        case KERNEL_CODE_SEGMENT:
            break;
        default:
            goto failed;
    }

    struct Message *msg = (struct Message *)trap_frame->rdi;
    switch (msg->src)
    {
        case Memory:
        case System:
        case Any:
            // TODO add some processes

            break;
        default:
            goto failed;
    }

    switch (msg->type)
    {
        case Send:
        case Receive:
            message_passing(msg, trap_frame);
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
    state      = init_task_call();
    if (!state)
    {
        return false;
    }

    // TODO setup system call
    return true;
}
