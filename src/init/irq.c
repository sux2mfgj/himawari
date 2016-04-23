#include <init.h>
#include <x86_64.h>
#include <descriptor.h>
#include <process.h>

// macro for PIC
#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_STATUS PIC_MASTER_COMMAND
#define PIC_MASTER_DATA PIC_MASTER_COMMAND + 1
#define PIC_MASTER_IMR PIC_MASTER_DATA
#define PIC_SLAVE_COMMAND 0xA0
#define PIC_SLAVE_STATUS PIC_SLAVE_COMMAND
#define PIC_SLAVE_DATA PIC_SLAVE_COMMAND + 1
#define PIC_SLAVE_IMR PIC_SLAVE_DATA

#define PIC_ICW1 0x11

#define PIC_MASTER_ICW3 0x04
#define PIC_SLAVE_ICW3 0x02

#define PIC_MASTER_ICW4 0x01
#define PIC_SLAVE_ICW4 0x01

#define PIC_IMR_MASK_IRQ0 0x01
#define PIC_IMR_MASK_IRQ1 (0x01 << 1)
#define PIC_IMR_MASK_IRQ2 (0x01 << 2)
#define PIC_IMR_MASK_IRQ3 (0x01 << 3)
#define PIC_IMR_MASK_IRQ4 (0x01 << 4)
#define PIC_IMR_MASK_IRQ5 (0x01 << 5)
#define PIC_IMR_MASK_IRQ6 (0x01 << 6)
#define PIC_IMR_MASK_IRQ7 (0x01 << 7)
#define PIC_IMR_MASK_ALL (0xFF)

// macro for PIT
#define PIT_REG_COUNTER0 0x0040
#define PIT_REG_COUNTER0 0x0040
#define PIT_REG_COUNTER1 0x0041
#define PIT_REG_COUNTER2 0x0042
#define PIT_REG_CONTROL 0x0043

/* Input CLK0		*/
#define DEF_PIT_CLOCK 1193181.67

#define DEF_PIT_COM_MASK_BINCOUNT 0x01
#define DEF_PIT_COM_MASK_MODE 0x0E
#define DEF_PIT_COM_MASK_RL 0x30
#define DEF_PIT_COM_MASK_COUNTER 0xC0

/* binary count */
#define DEF_PIT_COM_BINCOUNT_BIN 0x00
#define DEF_PIT_COM_BINCOUNT_BCD 0x01

/* counter mode */
#define DEF_PIT_COM_MODE_TERMINAL 0x00
#define DEF_PIT_COM_MODE_PROGONE 0x02
#define DEF_PIT_COM_MODE_RATEGEN 0x04
#define DEF_PIT_COM_MODE_SQUAREWAVE 0x06
#define DEF_PIT_COM_MODE_SOFTTRIG 0x08
#define DEF_PIT_COM_MODE_HARDTRIG 0x0A

/* data transfer */
#define DEF_PIT_COM_RL_LATCH 0x00
#define DEF_PIT_COM_RL_LSBONLY 0x10
#define DEF_PIT_COM_RL_MSBONLY 0x20
#define DEF_PIT_COM_RL_DATA 0x30

/* counter	 */
#define DEF_PIT_COM_COUNTER0 0x00
#define DEF_PIT_COM_COUNTER1 0x40
#define DEF_PIT_COM_COUNTER2 0x80

#define define_interrupt_handler(num, addr)

extern void timer_handler(void);

static bool test_switch = false;
void irq(int idt_entry_num)
{
    switch (idt_entry_num)
    {
        case IDT_ENTRY_PIC_TIMER:
            /*             puts("timer!"); */
            // schedule();
            if (!test_switch)
            {
                test_switch = true;
                start_first_task();
            }
            break;
        default:
            puts("who are you?????");
            while (true)
            {
            }
            break;
    }

    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

bool init_pit(void)
{
    uint32_t freq   = 100; // Hz
    uint16_t count  = (uint16_t)(DEF_PIT_CLOCK / freq);
    uint8_t command = DEF_PIT_COM_MODE_SQUAREWAVE | DEF_PIT_COM_RL_DATA |
                      DEF_PIT_COM_COUNTER0;

    outb(PIT_REG_CONTROL, command);

    outb(PIT_REG_COUNTER0, (uint8_t)(count & 0xff));
    outb(PIT_REG_COUNTER0, (uint8_t)((count >> 8) & 0xff));

    set_intr_gate(IDT_ENTRY_PIC_TIMER, &timer_handler);

    return true;
}

bool init_pic(void)
{
    outb(PIC_MASTER_COMMAND, PIC_ICW1);
    outb(PIC_SLAVE_COMMAND, PIC_ICW1);

    outb(PIC_MASTER_DATA, IDT_ENTRY_PIC_MASTER);
    outb(PIC_SLAVE_DATA, IDT_ENTRY_PIC_SLAVE);

    outb(PIC_MASTER_DATA, PIC_MASTER_ICW3);
    outb(PIC_SLAVE_DATA, PIC_SLAVE_ICW3);

    outb(PIC_MASTER_DATA, PIC_MASTER_ICW4);
    outb(PIC_SLAVE_DATA, PIC_SLAVE_ICW4);

    outb(PIC_MASTER_IMR, (~PIC_IMR_MASK_IRQ0) & (~PIC_IMR_MASK_IRQ2));
    // outb(PIC_MASTER_IMR, PIC_IMR_MASK_ALL);
    outb(PIC_SLAVE_IMR, PIC_IMR_MASK_ALL);

    init_pit();

    return true;
}