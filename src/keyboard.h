#ifndef _INCLUDED_KEYBOARD_H_
#define _INCLUDED_KEYBOARD_H_

#include "lib.h"

typedef struct {
    node* queue;
    uint32_t size;
}interrupt_queue;

static char key_table[] = {
    '0', '0', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9','-','=','0',
    '0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',']','0','0',
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\\','-','=','z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '0', '*', '0',' ','0','0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '7', '8','9','-','4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

bool keyboard_data_queue_check(void);
static interrupt_queue keyboard_data_queue;
void init_inthandler(void);
void keyboard_interrupt(void);

#endif
