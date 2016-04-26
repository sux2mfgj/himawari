#pragma once

// global descriptor table
#define NULL_SEGMENT 0x0
#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define SERVER_CODE_SEGMENT (0x18 | 0x1)
// #define SERVER_CODE_SEGMENT 0x18
#define SERVER_DATA_SEGMENT (0x20 | 0x1)
// #define SERVER_DATA_SEGMENT 0x20
#define USER_CODE_SEGMENT (0x28 | 0x03) //RPL 3
// #define USER_CODE_SEGMENT 0x28 
#define USER_DATA_SEGMENT (0x30 | 0x03) //RPL 3
// #define USER_DATA_SEGMENT 0x30 
#define TASK_STATE_SEGMENT 0x38



