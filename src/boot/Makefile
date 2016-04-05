TARGET := kernel.elf

MULTIBOOT_OBJ	:= boot_header.o
OBJS	:= $(MULTIBOOT_OBJ)

MEMMAP		:= kernel.map
LD_SCRIPT	:= link.ld
LDFLAGS		:= -T$(LD_SCRIPT) -Map $(MEMMAP) -nostdlib 
LDFLAGS 	+= -z max-page-size=0x1000

INCLUDE	:= -I./
ASM_CFLAGS := $(INCLUDE)

.SUFFIXES: .o .S
.S.o:
	$(CC) $(ASM_CFLAGS) -c -o $@  $<

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS) $(LD_SCRIPT) Makefile
	$(LD) $(LDFLAGS) $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET)
