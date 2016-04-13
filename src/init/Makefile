TARGET := init.o

OBJS	:= main.o early_memory.o pagetable.o vram_text.o trap.o

INCLUDE	:= -I../include

.PHONY: all
all: $(TARGET)

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@  $<

$(TARGET): $(OBJS) Makefile
	$(LD) $(LDFLAGS) -r $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET)
