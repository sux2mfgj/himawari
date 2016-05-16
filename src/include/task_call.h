#pragma once

#include <stdint.h>

enum ServerType {
    Memory,
//    Process,
    ServerNum,
    System,
    Any, 
};

enum MessageType {
    Send,
    Receive,
    Echo,
};

struct Content {
    union {
        struct {
            uintptr_t address;
            uint64_t num;
        };
        struct {
            uintptr_t start_address;
            uintptr_t end_address;
        };
    };
};

enum TaskCallType {
    Initialize = 0,
};

struct Message {
    union {
        enum ServerType dest;
        enum ServerType src;
    };
    enum TaskCallType number;
    enum MessageType type;
    struct Content content;
};

struct MessageInfo {
    struct Message buf;
    enum ServerType self;
    struct Message* addr;
};

extern uint64_t receive(enum ServerType src, struct Message *msg);
extern uint64_t send(enum ServerType dest, struct Message *msg);

