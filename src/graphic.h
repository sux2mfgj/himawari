#ifndef _INCLUDED_GRAPHIC_H_
#define _INCLUDED_GRAPHIC_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>


#define VRAM_TEXTMODE 0x000B8000

#define BLACK 0x0
#define WHITE 0xf

#define TEXT_MODE_WIDTH     0x0128 //80
#define TEXT_MODE_HEIGHT    0x0019 //25

#define TEXT_MODE_SCREEN_LEFT   0x00000000
#define TEXT_MODE_SCREEN_RIGHT  0x00000028

#define TEXT_MODE
// #define GRAPHIC_MODE

void init_screen(void);

void display_textmode(char c, uint8_t fore_color , uint8_t back_color
        , uint32_t x, uint32_t y);

void textmode_putc(char c, uint32_t x, uint32_t y, uint32_t place);
uint32_t textmode_puts(char* text, uint32_t x, uint32_t y, uint32_t place);
uint32_t integer_puts(uint32_t number, uint32_t x, uint32_t y, uint32_t place);
void printf(uint32_t place, char* format, ...);

void slide_screen(uint32_t place);




#endif
