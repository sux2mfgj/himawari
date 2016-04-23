#include <init.h>

static const uint16_t max_collum = 80;
// static const uint16_t max_row    = 25;
uint16_t vram_position = 0;

void putc(const char c)
{
    uint16_t *vram_tmode = (uint16_t *)0xffffffff800b8000;

    if ('\n' == c)
    {
        vram_position =
            (vram_position + max_collum) - (vram_position % max_collum);
        return;
    }

    vram_tmode[vram_position++] = (0x07 << 8) | c;
}

void puts(const char *text)
{
    for (int i = 0; text[i] != '\0'; ++i)
    {
        putc(text[i]);
    }
}
