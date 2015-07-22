#ifndef _INCLUDED_SYSTEM_H_
#define _INCLUDED_SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>
void initalize(void);
void system(void);

//test
bool test_count(int);

extern uint32_t system_stack_start;

#endif
