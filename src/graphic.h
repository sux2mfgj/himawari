#ifndef _INCLUDED_GRAPHIC_H_
#define _INCLUDED_GRAPHIC_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>


#define VRAM_TEXTMODE 0xc00B8000

#define BLACK 0x0
#define WHITE 0xf

#define TEXT_MODE_WIDTH     0x0050 //80
#define TEXT_MODE_HEIGHT    0x0019 //25

#define TEXT_MODE_SCREEN_LEFT   0x00000000
#define TEXT_MODE_SCREEN_RIGHT  0x00000028

#define TEXT_MODE
// #define GRAPHIC_MODE

typedef enum printk_place{
    PHYSICAL_MEM_SIZE,
    MAX_KERNEL_HEAP,
    FREE_KERNEL_HEAP,
    MAX_PAGE_NUM,
    FREE_PAGE_NUM,
    DEBUG1,
    DEBUG2,
    DEBUG3
}printk_place;
// #define PRINT_PLACE_PHYSI_MEM_SIZE 0
// #define PRINT_PLACE_MAX_KERNEL_HEAP 1
// #define PRINT_PLACE_FREE_KERNEL_HEAP 2
// #define PRINT_PLACE_MAX_PAGE_SIZE 3
// #define PRINT_PLACE_FREE_PAGE_SIZE 4

void init_screen(void);

void display_textmode(char c, uint8_t fore_color , uint8_t back_color
        , uint32_t x, uint32_t y);

static void textmode_putc(char c, uint32_t x, uint32_t y, uint32_t place);
static uint32_t textmode_puts(char* text, uint32_t x, uint32_t y, uint32_t place);
static uint32_t integer_puts(uint32_t number, uint32_t x, uint32_t y, uint32_t place);
static uint32_t hexadecimal_put(uint32_t number, uint32_t x, uint32_t y, uint32_t place);

void printf(uint32_t place, char* format, ...);
void printk(uint32_t print_place, char* format, ...);

void slide_screen(uint32_t place);




#endif
