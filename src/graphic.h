#ifndef _INCLUDED_GRAPHIC_H_
#define _INCLUDED_GRAPHIC_H_

#define VRAM_TEXTMODE 0x000B8000

#define BLACK 0x0
#define WHITE 0xf

#define PUTS_LEFT   0x00000000  // 0
#define PUTS_RIGHT  0x00000028  // 40

#define TEXT_MODE       0x0
// #define GRAPHIC_MODE    0x1

void display_textmode(char c, unsigned char fore_color
        , unsigned char back_color
        , int x, int y);

void textmode_puts(char* text, int y, unsigned int place);
void puts(char* text, unsigned int place);

void integer_puts(unsigned int number, int y, unsigned int place);




#endif
