
#include <list.h>
#include <stdlib.h>

struct linked_list* list_init(struct linked_list* ptr)
{
    if(ptr == NULL)
    {
        return NULL;
    }
    ptr->next = ptr;
    ptr->prev = ptr;

    return ptr;
}

struct linked_list* enqueue(struct linked_list* head, struct linked_list* new)
{
    if(head == NULL)
    {
        return NULL;
    }

    struct linked_list* head_prev = head->prev;

    head_prev->next = new;
    new->prev = head_prev;

    new->next = head;
    head->prev = new;

    return head;
}

struct linked_list* dequeue(struct linked_list* entry)
{
    if(entry == NULL) 
    {
        return NULL;
    }

    // entry only
    if(entry->next == entry)
    {
        return NULL;             
    }

    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;

    return entry->next;
}


