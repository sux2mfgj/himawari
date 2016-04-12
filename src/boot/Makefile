TARGET := kernel.elf

MULTIBOOT_OBJ	:= boot_header.o
OBJS	:= $(MULTIBOOT_OBJ) ../init/kernel.o

MEMMAP		:= kernel.map
LD_SCRIPT	:= link.ld
LDFLAGS		:= -T$(LD_SCRIPT) -Map $(MEMMAP) -nostdlib -nostdinc
LDFLAGS 	+= -z max-page-size=0x1000 

INCLUDE	:= -I../include
ASM_CFLAGS := $(INCLUDE) $(CFLAGS)

.PHONY: all
all: $(TARGET)

$(LD_SCRIPT): $(LD_SCRIPT).S 
	$(CC) -E $(INCLUDE) $< | grep -v '^#' > $@

.SUFFIXES: .o .S
.S.o:
	$(CC) $(ASM_CFLAGS) -c -o $@  $<

$(TARGET): $(OBJS) $(LD_SCRIPT) Makefile
	$(LD) $(LDFLAGS) $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET) $(LD_SCRIPT)
