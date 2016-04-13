#include <init.h>

static const uint16_t max_collum = 80;
static const uint16_t max_row    = 25;
uint16_t vram_position = 0;

void putc(const char c)
{
    uint16_t* vram_tmode = (uint16_t*)0x000b8000;

    if('\n' == c)
    {
        vram_position = (vram_position + 80) - (vram_position % 80);
        return;
    }

    vram_tmode[vram_position++] = (0x07 << 8) | c;
}

bool itoa(
        uint64_t num,
        char* buf,
        const uint64_t decimal)
{
    const int size = 32;
    char tmp[size];
    int next_pos = 0;

    static const char* trans_table = "0123456789ABCDEF";

    for(int i=0; i<size; ++i)
    {
        int dig = num % decimal;
        if(decimal == 10){
            tmp[next_pos++] = '0' + dig;
        }
        else if(decimal == 0x10)
        {
            tmp[next_pos++] = trans_table[dig];
        }
        else {
            return false;
        }
        
        num /= decimal;
        if(num == 0) {
            break;
        }
    }

    for(int i=0; i<next_pos; ++i)
    {
        buf[i] = tmp[next_pos - i - 1];
    }
    buf[next_pos] = '\0';

    return true;
}

void puts(const char const* text)
{
    for(int i=0; text[i] != '\0'; ++i)
    {
        putc(text[i]);
    }
}


