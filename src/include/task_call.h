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
    uint64_t address;
    uint64_t num;
};

enum TaskCallType {
    Regist = 0,
    Initialize = 1,
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

