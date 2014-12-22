#include "keyboard.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


void init_keyboard(void)
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

void keyboard_interrupt(void)
{
    unsigned char data;
    node *tmp;
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


