#include "graphic.h"
#include "kernel.h"
#include <stdarg.h>

bool init_text_screen(void)
{
    for (int i = 0; i < TEXT_MODE_WIDTH; ++i) {
        for (int j = 0; j < TEXT_MODE_HEIGHT; j++) {
            hputc(' ', i, j);
        }
    }
    printk = &hprintf;

    return true;
}

void display_char(const char c,
                  const text_color fore_color,
                  const text_color back_color,
                  const uint32_t x,
                  const uint32_t y)
{
    uint16_t color = (back_color << 4) | (fore_color & 0x0f);
    uint16_t* vram_write_addr = (uint16_t*)vram_textmove_addr;

    vram_write_addr += x + y * TEXT_MODE_WIDTH;
    *vram_write_addr = (color << 8) | c;

    return;
}

void hputc(const char c, const uint32_t x, const uint32_t y)
{
    display_char(c, WHITE, BLACK, x, y);
    return;
}


uint32_t hputs(const char* const text, const uint32_t x, const uint32_t y)
{
    int char_position = 0;
    while (text[char_position] != '\0' && (x + char_position) <= TEXT_MODE_WIDTH) {
        hputc(text[char_position], x + char_position, y);
        ++char_position;
    }

    return char_position;
}

static uint32_t integer_puts(uint32_t number,
                             const uint32_t x,
                             const uint32_t y)
{
    int string_length = 0;
    char buffer[TEXT_MODE_WIDTH];

    for (int i = 0; i < TEXT_MODE_WIDTH; ++i) {
        buffer[i] = '\0';
    }

    if (number == 0) {
        buffer[0] = '0';
        ++string_length;
    }

    while (number != 0) {
        int n = number % 10 + '0';
        buffer[string_length] = n;
        ++string_length;
        number /= 10;
    }

    for (int i = 0; i < string_length; ++i) {
        hputc(buffer[string_length - i - 1], i + x, y);
    }

    return string_length;
}

static uint32_t hexadecimal_puts(uint32_t number,
                                 const uint32_t x,
                                 const uint32_t y)
{
    int string_length = 0;
    char buffer[TEXT_MODE_WIDTH];

    const char hex[] = {'0',
                        '1',
                        '2',
                        '3',
                        '4',
                        '5',
                        '6',
                        '7',
                        '8',
                        '9',
                        'A',
                        'B',
                        'C',
                        'D',
                        'E',
                        'F'};

    for (int i = 0; i < TEXT_MODE_WIDTH; ++i) {
        buffer[i] = '\0';
    }

    if (number == 0) {
        buffer[0] = '0';
        ++string_length;
    }

    while (number != 0) {
        int n = number % 16;
        buffer[string_length] = hex[n];
        ++string_length;
        number /= 16;
    }

    for (int i = 0; i < string_length; ++i) {
        hputc(buffer[string_length - i - 1], i + x, y);
    }

    return string_length;
}


//TODO:ptsをつくったら関数ポインタでprintfを繋ぎ変えてそっちを使うようにする
bool hprintf(const char* const format, ...)
{
    const char *f;
    va_list args;
    va_start(args, format);
    uint32_t x = 0;

    if(print_line_number > TEXT_MODE_HEIGHT){
        slide_screen();
    }

    for(f = format; *f != '\0'; ++f){
        if('%' == *f){
            ++f;
            switch(*f) {
                case 'c':
                    hputc(va_arg(args, int), x++, print_line_number);
                    break;

                case 's':
                    x += hputs(va_arg(args, char *), x, print_line_number);
                    break;

                case 'd':
                    x += integer_puts(va_arg(args, int), x, print_line_number);
                    break;

                case 'x':
                    x += hexadecimal_puts(va_arg(args, int), x, print_line_number);
                    break;

                default:
                    hputs(" print error", x, ++print_line_number);
                    goto finish;
            }
        }
        else {
            hputc(*f, x++, print_line_number);
        }
    }

finish:

    if (TEXT_MODE_HEIGHT > print_line_number) {
        ++print_line_number;
    }

    va_end(args);
    return 0;
}

static void slide_screen(void)
{
    for (int i = 0; i <= TEXT_MODE_HEIGHT; ++i) {
        for (int j = 0; j < TEXT_MODE_WIDTH; ++j) {
            const uintptr_t read_addr =
                vram_textmove_addr + i * TEXT_MODE_WIDTH + j;
            uint16_t* const write_addr = (uint16_t*)read_addr - TEXT_MODE_WIDTH;
            *write_addr = (WHITE << 8) | *(uint16_t*)read_addr;
        }
    }

    for (int i = 0; i < TEXT_MODE_WIDTH; ++i) {
        hputc(' ', i, TEXT_MODE_HEIGHT);
    }

    return;
}
