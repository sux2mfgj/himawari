TARGET	:= kernel.elf

MEMMAP	:= memory.map

LD_SCRIPT	:= kernel.ld
LD_SCRIPT_S	:= kernel.ld.S

LD_FLAGS	:= -Map $(MEMMAP) -T $(LD_SCRIPT) -nostdlib

INCLUDE	:= -I../include
CFLAGS += $(INCLUDE)

OBJS	:= entry.o pci.o uefi.o main.o acpi.o ../libs/string.o utils.o

all: $(TARGET)

.SUFFIXES: .c.o
%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

.SUFFIXES: .s.o
%o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) $(LD_SCRIPT) Makefile
	$(LD) $(LD_FLAGS) $(OBJS) -o $@

$(LD_SCRIPT): $(LD_SCRIPT_S)
	$(CC) -E $< | grep -v '^#' > $@

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET)
