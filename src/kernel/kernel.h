#ifndef _INCLUDED_MAIN_H_
#define _INCLUDED_MAIN_H_

#include <stddef.h>

#include "multiboot.h"

void kernel_entry(const uint32_t magic, const multiboot_info *mb_info);

bool (*printk)(const char* const format, ...);

#define kernel_panic(msg) do{kernel_panic_handler(msg, __FILE__, __LINE__);} while(false);
void kernel_panic_handler(const char* name, const char* file, const int line);

void start_tasks(uint32_t* task_esp);

extern char _kernel_start, _kernel_end, _text_start, _text_end, _data_start, _data_end, _bss_start, _bss_end;

#endif
