#pragma once

#include <task_call.h>
#include <stdbool.h>

extern bool (*init_task_table[ServerNum])(struct Message* msg);

// server initialization
extern bool memory_server_init(struct Message* msg);
