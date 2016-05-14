#include <system.h>
#include <string.h>
#include <early_memory.h>


//  1, receive message for initialize from memory server.
//      (in system task)
//
//  (this function do from here)
//  2, copy bitmap to memory server space.
//      
//  3, notify server of completion that is copy bitmap.
//  the message should contain new bitmap size
//      (by send message)
bool memory_server_init(struct Message* msg)
{

    uintptr_t dest_base_addr = msg->content.address;
    int size = msg->content.num; 
    int early_mem_bitmap_size = (bitmap_size * sizeof(uint64_t));

    if(size < early_mem_bitmap_size)
    {
        return false;
    }

    memcpy((void *)dest_base_addr, (void *)bitmap, bitmap_size);

    struct Message send_msg = {
        .number = Initialize,
        .content = {
            .address = 0,
            .num = early_mem_bitmap_size,
        },
    };

    uint64_t res = send(System, &send_msg);

    return true;
}

