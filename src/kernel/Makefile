CC := clang
AS := as
LD := ld

CFLAGS := -ggdb3 -Wall --target=i686-elf -m32
ASFLAGS := --32
PPFLAGS := -E

MAPFILE := kernel.map
LD_SCRIPT := kernel.ld

LDFLAGS := -m elf_i386 --format elf32-i386 --oformat elf32-i386 \
		  -Map $(MAPFILE) -T$(LD_SCRIPT) -nostdlib

C_OBJS := segment.o main.o graphic.o trap.o kmemory.o
ASM_OBJS := entry.o functions.o vectors.o

OBJS := $(ASM_OBJS) $(C_OBJS)

IMAGE := himawari
DUMP_FILE := dump

.SUFFIXES: .o .s .S .c
.s.o:
	$(AS) $(ASFLAGS) -o $@ $<
.S.o:
	$(CC) $(PPFLAGS) $< > $(join $<, .tmp)
	$(AS) $(ASFLAGS) -o $@ $(join $<, .tmp)
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: all
all: $(IMAGE) $(DUMP_FILE)

$(IMAGE): $(OBJS) $(LD_SCRIPT)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(DUMP_FILE): $(IMAGE)
	objdump -d $(IMAGE) > $(DUMP_FILE)

clean:
	rm -f *.o *.tmp $(IMAGE)

