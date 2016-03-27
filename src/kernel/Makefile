TARGET	:= kernel.efi

MEMMAP	:= memory.map

LD_SCRIPT	:= kernel.ld
LDFLAGS	+= -Map $(MEMMAP) -T $(LD_SCRIPT)

INIT	:= main.o
INIT_OBJS	:= $(foreach file, $(INIT), init/$(file))

OBJS	:= $(INIT_OBJS)

all: $(TARGET)

.SUFFIXES: .c.o
%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) $(LD_SCRIPT) 
	$(LD) $(LDFLAGS) $(OBJS) -o $@

.PHONY: clean
clean:
	$(RM) $(OBJS)
