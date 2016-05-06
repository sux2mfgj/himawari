#pragma once

enum ServerType {
//    Memory,
//    Process,
    ServerNum,
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
