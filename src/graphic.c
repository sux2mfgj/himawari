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
    color = (back_color << 4) | (fore_color & 0x0f);

    vram_textmode += x + y * 80;

    *vram_textmode = (color << 8) | c;

}

void textmode_puts(char* text, int y, unsigned int place)
{
    int i = 0;
    char t;
    while(text[i] != '\0'){
        display_textmode(text[i] , WHITE, BLACK , i + place, y);
        i++;
    }
}

void integer_puts(unsigned number, int y, unsigned int place)
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
        display_textmode( buf[i-j-1], WHITE, BLACK, j + place, y);
    }

    return;
}

void puts(char* text, unsigned int place)
{
    static int left_line_num = 0;
    static int right_line_num = 0;
    //TODO: add code about when now_line over 80
#ifdef TEXT_MODE
    if (place == PUTS_RIGHT) {
        textmode_puts(text, right_line_num++, place);
    }
    else {
        textmode_puts(text, left_line_num++, place);
    }
#elif GRAPHIC_MODE

#endif
}


