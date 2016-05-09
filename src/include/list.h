#pragma once

#include <stdbool.h>

struct linked_list {
    struct linked_list* next;
    struct linked_list* prev;
};

//extern bool enqueue(struct linked_list* head, struct linked_list* new);
//extern bool dequeue(struct linked_list* head, struct linked_list* entry);
extern struct linked_list* enqueue(struct linked_list* head, struct linked_list* new);
extern struct linked_list* dequeue(struct linked_list* entry);
//extern bool list_init(struct linked_list* ptr);
extern struct linked_list* list_init(struct linked_list* ptr);
