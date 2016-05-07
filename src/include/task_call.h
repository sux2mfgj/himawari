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

struct Message {
    enum ServerType source;
    enum ServerType dest;
    enum MessageType type;
    struct Content content;
};

extern uint64_t receive(struct Message *msg);
extern uint64_t send(struct Message *msg);

