#include"interrupt_handler.h"
#include "segment.h"
#include "task.h"
#include "graphic.h"

static uint32_t timer_tick = 0;
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
/*     io_out8(0xa0, 0x20); */
    timer_tick++;
/*     if (timer_tick == 100) { */
/*        task_switch_c(0, 1); */
/*     } */
/*     else if (timer_tick == 200) { */
/*         task_switch_c(1, 2); */
/*     } */

/*     printf(TEXT_MODE_SCREEN_RIGHT, "timer: %d", timer_tick); */
    return;
}

void keyboard_inthandler(int *esp)
{
    static int n = 0;
    unsigned char data;
    node* tmp;
/*     io_out8(PIC_MASTER_CMD_STATE_PORT, 0x61); */
    io_out8(PIC_MASTER_CMD_STATE_PORT, PIC_OCW2_EOI);
    data = io_in8(0x0060);
/*     printf(TEXT_MODE_SCREEN_RIGHT, "%d", data); */


/*     if (a == 1) { */
/*         a = 0; */
/*         b = 1; */
/*     } */
/*     else { */
/*         a = 1; */
/*         b = 0; */
/*     } */
    if (data <= 81) {

/*         if (n == 0) { */
/*         ++n; */
/*     printf(TEXT_MODE_SCREEN_RIGHT, "interrupt success 1"); */
/*             task_switch_c(0, 1); */
/*         } */
/*         else if((n%2)!=0){ */
/*         ++n; */
/*     printf(TEXT_MODE_SCREEN_RIGHT, "interrupt success 2"); */
/*             task_switch_c(1, 2); */
/*         } */
/*         else { */
/*         ++n; */
/*     printf(TEXT_MODE_SCREEN_RIGHT, "interrupt success 1"); */
/*             task_switch_c(2,1); */
/*         } */

        tmp = new_node(sizeof(char));
        *(char *)(tmp->data) = key_table[data];
        if (keyboard_data_queue.size == 0) {
            keyboard_data_queue.queue = tmp;
        }
        else {
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

void page_fault_handler(int *esp)
{
    printf(TEXT_MODE_SCREEN_RIGHT, "in page fault handler");
    io_hlt();
}
