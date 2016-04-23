TARGET := kernel.elf

MULTIBOOT_OBJ	:= boot_header.o pre_kernel.o
FIXED_LINK_OBJS := $(foreach objs, $(LINK_OBJS), ../$(objs))
OBJS	:= $(MULTIBOOT_OBJ) $(FIXED_LINK_OBJS) #../init/init.o ../lib/lib.o 

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

.SUFFIXES: .o .S .c
.S.o:
	$(CC) $(ASM_CFLAGS) -c -o $@  $<

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

$(TARGET): $(OBJS) $(LD_SCRIPT) Makefile
	$(LD) $(LDFLAGS) $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET) $(LD_SCRIPT)
