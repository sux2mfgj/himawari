TARGET := init.o

# remove local_apic.o
OBJS	:= main.o early_memory.o pagetable.o vram_text.o trap.o entry.o irq.o process.o elf.o

INCLUDE	:= -I../include

.PHONY: all
all: $(TARGET)

.SUFFIXES: .o .c .S
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<
.S.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

$(TARGET): $(OBJS) Makefile
	$(LD) $(LDFLAGS) -r $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET) *.o
