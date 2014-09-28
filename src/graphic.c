#include "graphic.h"
#include "lib.h"

static uint32_t left_line_num = 0;
static uint32_t right_line_num = 0;

void init_screen()
{
#ifdef TEXT_MODE
    int i, j;
    for (i = 0; i < TEXT_MODE_WIDTH; ++i) {
        for (j = 0; j < TEXT_MODE_HEIGHT; j++) {
            display_textmode(' ', WHITE, BLACK, i, j);
        }
    }

#elif GRAPHIC_MODE

#endif
    return;
}

void display_textmode(char c, uint8_t fore_color, uint8_t back_color,
                      uint32_t x, uint32_t y)
{
    uint16_t *vram_textmode;
    uint16_t color;

    vram_textmode = (uint16_t *)VRAM_TEXTMODE;
    color = (back_color << 4) | (fore_color & 0x0f);

    vram_textmode += x + y * 80;

    *vram_textmode = (color << 8) | c;

    return;
}

static void textmode_putc(char c, uint32_t x, uint32_t y, uint32_t place)
{
    display_textmode(c, WHITE, BLACK, place + x, y);
}

static uint32_t textmode_puts(char *text, uint32_t x, uint32_t y,
                              uint32_t place)
{
    int i = 0;
    char t;
    while (text[i] != '\0') {
        display_textmode(text[i], WHITE, BLACK, i + place + x, y);
        i++;
    }

    return i;
}

static uint32_t integer_puts(uint32_t number, uint32_t x, uint32_t y,
                             uint32_t place)
{
    int i = 0, j;
    char buf[20];

    memset(buf, '\0', 20);

    if (number == 0) {
        buf[0] = '0';
        ++i;
    }

    while (number != 0) {
        int n = number % 10 + 48;
        buf[i] = n;
        i++;
        number /= 10;
    }

    for (j = 0; j < i; j++) {
        display_textmode(buf[i - j - 1], WHITE, BLACK, j + place + x, y);
    }

    return i;
}

static uint32_t hexadecimal_put(uint32_t number, uint32_t x, uint32_t y,
                                uint32_t place)
{
    // TODO: make function

    int i = 0;
    char buf[20];
    char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    memset(buf, '\0', 20);

    if (number == 0) {
        buf[0] = '0';
        ++i;
    }

    while (number != 0) {
        int n = number % 16;
        buf[i] = hex[n];
        ++i;
        number /= 16;
    }

    for (int j = 0; j < i; ++j) {
        display_textmode(buf[i - j - 1], WHITE, BLACK, j + place + x, y);
    }

    return i;
}

/* void puts(char* text, uint32_t place) */
/* { */
/*     static int left_line_num = 0; */
/*     static int right_line_num = 0; */
/*     //TODO: add code about when now_line over 80 */
/* #ifdef TEXT_MODE */
/*     if (place == TEXT_MODE_SCREEN_LEFT) { */
/*         textmode_puts(text, right_line_num++, place); */
/*     } */
/*     else { */
/*         textmode_puts(text, left_line_num++, place); */
/*     } */
/* #elif GRAPHIC_MODE */

/* #endif */
/* } */

void slide_screen(uint32_t place)
{
    uint16_t *vram_textmode = (uint16_t *)VRAM_TEXTMODE;
    int i, j;
    char tmp;
    uint16_t *read_addr;

    for (i = 0; i <= TEXT_MODE_HEIGHT; i++) {
        for (j = 0; j < (TEXT_MODE_WIDTH / 2); j++) {
            read_addr = vram_textmode + i * TEXT_MODE_WIDTH + j + place;
            tmp = (char)*read_addr;
            uint16_t* write_addr = read_addr - TEXT_MODE_WIDTH;
            *write_addr = (WHITE << 8) | tmp;
        }
    }

    for (i = 0; i < (TEXT_MODE_WIDTH / 2); i++) {
        display_textmode(' ', WHITE, BLACK, i + place, TEXT_MODE_HEIGHT);
    }

    return;
}

void printk(uint32_t print_place, char *format, ...)
{
    char *f;
    va_list args;
    va_start(args, format);
    uint32_t x = 0;

    for (f = format; *f != '\0'; ++f) {
        if (*f == '%') {
            f++;

            switch (*f) {
                case 'c':
                    textmode_putc((uint8_t)va_arg(args, int), x++, print_place,
                                  TEXT_MODE_SCREEN_LEFT);
                    break;
                case 's':
                    x += textmode_puts((char *)va_arg(args, char *), x,
                                       print_place, TEXT_MODE_SCREEN_LEFT);
                    break;

                case 'd':
                    x += integer_puts((uint32_t)va_arg(args, int), x,
                                      print_place, TEXT_MODE_SCREEN_LEFT);
                    break;

                case 'x':
                    x += hexadecimal_put((uint32_t)va_arg(args, int), x,
                                         print_place, TEXT_MODE_SCREEN_LEFT);
                    break;
            }

        } else {
            textmode_putc((uint8_t) * f, x++, print_place, TEXT_MODE_SCREEN_LEFT);
        }
    }

    va_end(args);
    return;
}

void printf(uint32_t place, char *format, ...)
{
    char *f;
    va_list args;
    va_start(args, format);
    uint32_t x = 0;
    uint32_t *y;

    if (place == TEXT_MODE_SCREEN_LEFT) {
        y = &left_line_num;
    } else {
        y = &right_line_num;
    }

    if (*y == 25) {
        slide_screen(place);
    } else {
        (*y)++;
    }

    for (f = format; *f != '\0'; ++f) {
        if (*f == '%') {
            f++;

            switch (*f) {
                case 'c':
                    textmode_putc((uint8_t)va_arg(args, int), x++, *y, place);
                    break;
                case 's':
                    x += textmode_puts((char *)va_arg(args, char *), x, *y,
                                       place);
                    break;

                case 'd':
                    x +=
                        integer_puts((uint32_t)va_arg(args, int), x, *y, place);
                    break;

                case 'x':
                    x += hexadecimal_put((uint32_t)va_arg(args, int), x, *y,
                                         place);
                    break;
            }

        } else {
            textmode_putc((uint8_t) * f, x++, *y, place);
        }
    }

    va_end(args);
    return;
}

