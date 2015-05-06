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

C_OBJS := main.o graphic.o segment.o
ASM_OBJS := entry.o functions.o

OBJS := $(ASM_OBJS) $(C_OBJS)

IMAGE := himawari

.SUFFIXES: .o .s .S .c
.s.o:
	$(AS) $(ASFLAGS) -o $@ $<
.S.o:
	$(CC) $(PPFLAGS) $< > $(join $<, .tmp)
	$(AS) $(ASFLAGS) -o $@ $(join $<, .tmp)
.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(IMAGE)

$(IMAGE): $(OBJS) $(LD_SCRIPT)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f *.o *.tmp $(IMAGE)

