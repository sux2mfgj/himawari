#include"interrupt_handler.h"

static uint32_t timer_tick = 0;
static interrupt_queue keyboard_data_queue;

void timer_inthandler(void)
{
    io_out8(0x20, 0x20);
    io_out8(0xa0, 0x20);
    timer_tick++;
/*     printf(TEXT_MODE_SCREEN_RIGHT, "timer: %d", timer_tick); */

    return;
}

void keyboard_inthandler(int *esp)
{
    unsigned char data;
    node* tmp;
/*     printf(TEXT_MODE_SCREEN_RIGHT, "interrupt success"); */
    io_out8(0x0020, 0x61);
    data = io_in8(0x0060);


    if (data <= 81) {
        tmp = new_node(sizeof(char));
        *(char *)(tmp->data) = key_table[data];
        if (keyboard_data_queue.size == 0) {
            keyboard_data_queue.queue = tmp;
        }
        else {
            append_node(keyboard_data_queue.queue, tmp);
/*             print_array_status(); */
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
    }
    else {
        printf(TEXT_MODE_SCREEN_LEFT, "key: %c", *(char *)keyboard_data_queue.queue->data);

        keyboard_data_queue.queue = delete_node(keyboard_data_queue.queue
                , keyboard_data_queue.queue);
        keyboard_data_queue.size--;
        return false;
    }
}

void test_keyboard_data_queue(void)
{
    if (keyboard_data_queue.size == 0) {
        keyboard_data_queue.queue = new_node(sizeof(unsigned char));
    }
    else {
        append_node(keyboard_data_queue.queue, new_node(sizeof(unsigned char)));
    }
    printf(TEXT_MODE_SCREEN_LEFT, "queue: %d", keyboard_data_queue.queue);
    print_array_status();
    keyboard_data_queue.queue = delete_node(keyboard_data_queue.queue,
            keyboard_data_queue.queue);
    if (keyboard_data_queue.queue == NULL) {
        printf(TEXT_MODE_SCREEN_LEFT, "NULL");
    }
    print_array_status();

}
