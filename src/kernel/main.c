#include <boot/multiboot2.h>

#include <stdint.h>
#include <stdbool.h>

enum mmap_type {
    RESERVED = 0,
    AVAILABLE = 1,
    ACPI = 3,
    //4?
};

static const uint16_t max_collum = 80;
static const uint16_t max_row    = 25;
uint16_t vram_position = 0;

#define ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)&~(mask)))
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

void start_kernel(uintptr_t bootinfo_addr) 
{

    uint32_t total_size = *(uint64_t *)bootinfo_addr;

    struct multiboot_tag *tag;


/*     uint32_t tag, size; */
    do{
        tag = (struct multiboot_tag*)(bootinfo_addr + 8);
/*         tag = *(uint32_t *)(bootinfo_addr + 8); */
/*         size = *(uint32_t *)(bootinfo_addr + 12); */

        uint32_t size = tag->size;
        //alignment size 8
        size = (size + 7) & ~7;

        switch(tag->type) {

            case MULTIBOOT_TAG_TYPE_MMAP:
            {

                multiboot_memory_map_t *mmap;
                for(
                    mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                    (uintptr_t)mmap < (uintptr_t)tag + tag->size;
                    mmap = (multiboot_memory_map_t *)((uintptr_t)mmap 
                        + ((struct multiboot_tag_mmap *)tag)->entry_size))
                {
                    switch(mmap->type) {
                        case MULTIBOOT_MEMORY_AVAILABLE:
                        {
                            puts("available: ");
                            uintptr_t addr = mmap->addr;
                            uint64_t len = mmap->len; 

                            char buf[32];
                            itoa(addr, buf, 16);
                            puts(buf);
                            puts(" - ");
                            itoa(len, buf, 16);
                            puts(buf);
                            puts("\n");
                            break;
                        }

                        case MULTIBOOT_MEMORY_RESERVED:
                        {
                            puts("reserved: ");
                            uintptr_t addr = mmap->addr;
                            uint64_t len = mmap->len; 
                            char buf[32];
                            itoa(addr, buf, 16);
                            puts(buf);
                            puts(" - ");
                            itoa(len, buf, 16);
                            puts(buf);
                            puts("\n");

                            break;
                        }

                        default:
                            break;
                    }

                }

                break;
            }

            default:
                break;
        }
        bootinfo_addr += size;

    } while(tag->type != MULTIBOOT_TAG_TYPE_END) ;

    while(1) {

    }
}
