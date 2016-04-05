TARGET	:= kernel.elf

MEMMAP	:= memory.map

INIT	:= main.o 
INIT_OBJS	:= $(foreach file, $(INIT), init/$(file))

LD_SCRIPT	:= kernel.ld
LD_SCRIPT_S	:= kernel.ld.S

LD_FLAGS	:= -Map $(MEMMAP) -T $(LD_SCRIPT) -nostdlib

INCLUDE	:= -I../include
CFLAGS += $(INCLUDE)

OBJS	:= $(INIT_OBJS)
MAIN_SO	:= main.so 

all: $(TARGET)

.SUFFIXES: .c.o
%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) $(LD_SCRIPT) Makefile
	$(LD) $(LD_FLAGS) $(OBJS) -o $@

$(LD_SCRIPT): $(LD_SCRIPT_S)
	$(CC) -E $< | grep -v '^#' > $@

.PHONY: clean
clean:
	$(RM) $(OBJS) $(MAIN_SO) $(TARGET)
