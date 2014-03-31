#include"graphic.h"

void display_textmode(char c
        , unsigned char fore_color
        , unsigned char back_color
        , int x
        , int y)
{
    unsigned short *vram_textmode;
    unsigned short color;

    vram_textmode = (unsigned short*)VRAM_TEXTMODE;
    color = (back_color << 4) 
                | (fore_color & 0x0f);
    
    vram_textmode += x + y * 80;

    *vram_textmode = (color << 8) | c;

}

void textmode_puts(char* text, int y)
{
    int i = 0;
    char t; 
    while(text[i] != '\0'){
        display_textmode(text[i]
                , WHITE, BLACK
                , i, y);  
        i++;
    }
}

