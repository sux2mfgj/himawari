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

void integer_puts(unsigned number, int y)
{
    int n, i = 0, j;
    char buf[20];
    for(i=0;i<20; i++){
        buf[i] = '\0';
    }
    i=0;

    while(number != 0){
        n = number%10+48;
        buf[i] = n;
        i++;
        number /= 10;
    }

    for(j=0; j<i; j++){
        display_textmode(
                buf[i-j-1],
                WHITE,
                BLACK,
                j,
                y);
    }


    return;
}

void h_puts(char* text)
{
    static int now_line = 0;
    textmode_puts(text, now_line++);
    //TODO: add code about when now_line over 80
}


