#ifndef _INCLUDED_GRAPHIC_H_
#define _INCLUDED_GRAPHIC_H_

#include <stdint.h>
#include <stdbool.h>

#include "paging.h"

#define VRAM_TEXTMODE 0x000B8000 + VIRTUAL_KERNEL_BASE_ADDR

#define TEXT_MODE_WIDTH 0x0050   // 80
#define TEXT_MODE_HEIGHT 0x0019  // 25

typedef enum text_mode_char_color{
    BLACK = 0x0,
    WHITE = 0xf,
}text_color;

static const uintptr_t vram_textmove_addr = VRAM_TEXTMODE;
static uint32_t print_line_number = 0;

bool init_screen(void);
void display_char(const char c,
                  const text_color fore_color,
                  const text_color back_color,
                  const uint32_t x,
                  const uint32_t y);

void hputc(const char c, const text_color x, const text_color y);
uint32_t hputs(const char* const text, const uint32_t x, const uint32_t y);
void hprintf(const char* const format, ...);

static uint32_t integer_puts(uint32_t number,
                             const uint32_t x,
                             const uint32_t y);
static uint32_t hexadecimal_puts(uint32_t number,
                                 const uint32_t x,
                                 const uint32_t y);

static void slide_screen(void);
#endif
