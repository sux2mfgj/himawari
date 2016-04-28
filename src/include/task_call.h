#pragma once

#include <process.h>
#include <kernel.h>

enum message_type
{
    SEND,
    RECEIVE,
    ECHO,
};

struct message
{
    enum task source;
    enum task destination;
    enum message_type type;
    int message_data_type;
    union {
        struct{

        } content_0;
    } content;
};

extern void task_call(struct trap_frame_struct *trap_frame);

