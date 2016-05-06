#pragma once

#include <stdint.h>

enum ServerType {
    Memory,
//    Process,
    ServerNum,
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
