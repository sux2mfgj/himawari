#ifndef _INCLUDED_LIB_H_
#define _INCLUDED_LIB_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include"memory.h"
#include"graphic.h"

typedef struct _node{
    struct _node *prev, *next;
    void *data;
}node;


node *new_node(uint32_t data_size);
bool append_node(node *list_head, node* append_node);
node *delete_node(node *list_head, node* del_node);

void list_test(void);



#endif
