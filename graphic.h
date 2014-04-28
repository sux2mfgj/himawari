#ifndef _INCLUDED_GRAPHIC_H_
#define _INCLUDED_GRAPHIC_H_

#define VRAM_TEXTMODE 0x000B8000

void display_textmode(char c, unsigned char fore_color
        , unsigned char back_color
        , int x, int y);

void textmode_puts(char* text, int y);
void h_puts(char* text);


#define BLACK 0x0
#define WHITE 0xf

#endif
