#include "interrupt_handler.h"
#include "segment.h"
#include "task.h"
#include "graphic.h"
#include "timer.h"
#include "task.h"

static interrupt_queue keyboard_data_queue;

void fault_inthandler(int *esp)
{
    printf(TEXT_MODE_SCREEN_RIGHT, "fault: %x", *esp);
}

void fault_inthandler2(int *esp)
{
    printf(TEXT_MODE_SCREEN_RIGHT, "fault2: %x", *esp);
}

void timer_inthandler(int *esp)
{
    io_out8(PIC_MASTER_CMD_STATE_PORT, PIC_OCW2_EOI);
    timer_interrupt();
    return;
}

void keyboard_inthandler(int *esp)
{
    static int n = 0;
    unsigned char data;
    node *tmp;
    io_out8(PIC_MASTER_CMD_STATE_PORT, PIC_OCW2_EOI);
    data = io_in8(0x0060);

    if (data <= 81) {
        tmp = new_node(sizeof(char));
        *(char *)(tmp->data) = key_table[data];
        if (keyboard_data_queue.size == 0) {
            keyboard_data_queue.queue = tmp;
        } else {
            append_node(keyboard_data_queue.queue, tmp);
        }
        keyboard_data_queue.size++;
    }

    return;
}

void init_inthandler(void)
{
    keyboard_data_queue.size = 0;
    keyboard_data_queue.queue = NULL;
}

bool keyboard_data_queue_check(void)
{
    if (keyboard_data_queue.size == 0) {
        return true;
    } else {
        printf(TEXT_MODE_SCREEN_RIGHT, "key: %c",
               *(char *)keyboard_data_queue.queue->data);

        keyboard_data_queue.queue =
            delete_node(keyboard_data_queue.queue, keyboard_data_queue.queue);
        keyboard_data_queue.size--;
        return false;
    }
}

void test_keyboard_data_queue(void)
{
    if (keyboard_data_queue.size == 0) {
        keyboard_data_queue.queue = new_node(sizeof(unsigned char));
    } else {
        append_node(keyboard_data_queue.queue, new_node(sizeof(unsigned char)));
    }
    printf(TEXT_MODE_SCREEN_RIGHT, "queue: %d", keyboard_data_queue.queue);
    print_array_status();
    keyboard_data_queue.queue =
        delete_node(keyboard_data_queue.queue, keyboard_data_queue.queue);
    if (keyboard_data_queue.queue == NULL) {
        printf(TEXT_MODE_SCREEN_RIGHT, "NULL");
    }
    print_array_status();
}

void page_fault_handler(int *esp)
{
    printf(TEXT_MODE_SCREEN_RIGHT, "in page fault handler");
    io_hlt();
}
