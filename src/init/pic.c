#include <init.h>
#include <x86_64.h>

#define PIC_MASTER_COMMAND  0x20
#define PIC_MASTER_DATA     PIC_MASTER_COMMAND + 1
#define PIC_SLAVE_COMMAND   0xA0
#define PIC_SLAVE_DATA      PIC_SLAVE_COMMAND + 1

bool init_pic(void)
{
    uint8_t master, slave;

    inb(PIC_MASTER_DATA, &master);
    inb(PIC_SLAVE_DATA, &slave);

    //outb(PIC_MASTER_COMMAND, )



}
