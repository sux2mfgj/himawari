#include"lib.h"


node *new_node(uint32_t data_size)
{
    node *ret = memory_allocate(sizeof(node));

    ret->data = memory_allocate(data_size);
    ret->next = NULL;
    ret->prev = NULL;

    return ret;
}




bool append_node(node *list_head, node* append_node)
{
    node *current_node = list_head;

    if (list_head == NULL) {
        return false;
    }

    while (current_node->next != NULL) {
        current_node = current_node->next;
    }

    current_node->next = append_node;
    append_node->prev = current_node;
    append_node->next = NULL;

    return true;
}

node *delete_node(node *list_head, node* del_node)
{
    node *current_node = list_head;

    node *tmp = list_head->next;
    printf(TEXT_MODE_SCREEN_LEFT, "list_head: %d", list_head);

    if (list_head == del_node) {
        if (!memory_free(list_head->data)){
            printf(TEXT_MODE_SCREEN_LEFT, "list_head->data free faild :%d", list_head->data);
        }
        if (!memory_free(list_head)){
            printf(TEXT_MODE_SCREEN_LEFT, "list_head free faild :%d",list_head);
        }
        if (tmp == NULL) {
            return NULL;
        }
        return tmp;
    }
    else {
        tmp = del_node->prev;
        tmp->next = del_node->next;
        del_node->next->prev = tmp;
        memory_free(del_node->data);
        memory_free(del_node);

        return list_head;
    }
}


void list_test(void)
{
    node *head;

    print_array_status();
    head = new_node(sizeof(uint32_t));
    print_array_status();
    append_node(head, new_node(sizeof(uint32_t)));
    print_array_status();

    head = delete_node(head, head);
    print_array_status();
/*     head = delete_node(head, head); */
/*     print_array_status(); */

}


