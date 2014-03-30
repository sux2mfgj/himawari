extern void start_hlt(void);

void kernel_entry()
{
    start_hlt();
}
