#include"graphic.h"
#include"segment.h"

extern void start_hlt(void);
extern void write_mem8(int , int);
extern void io_cli(void);
extern void io_sti(void);

void h_puts(char* text);

void kernel_entry()
{
    io_cli();
    init_gdtidt();
    init_pic();
    io_sti();
    
    int now_line = 0;
    h_puts("hello");

    for(;;){
        io_hlt();
    }
}

void h_puts(char* text)
{
    static int now_line = 0;
    textmode_puts(text, now_line++);
    //TODO: add code about when now_line over 80
}

void inthandler21(int *esp)
{
/*     h_puts("hello"); */
    h_puts("interrupt success");
/*     io_hlt(); */
}
