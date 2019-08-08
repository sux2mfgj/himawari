TARGET	:= BOOTX64.efi

MEMMAP	:= memory.map

INCLUDE	:= -I../include
EFI_CFLAGS += $(INCLUDE)
#  CFLAGS += $(INCLUDE)

OBJS	:= main.o ../libs/elf.o
MAIN_SO	:= main.so

all: $(TARGET)

main.o: main.c Makefile
	$(CC) $< $(EFI_CFLAGS) $(EFI_INCLUDES) -c -o $@

.SUFFIXES: .c.o
%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(MAIN_SO)
	$(OBJCOPY) 					\
		-j .text                \
		-j .sdata               \
		-j .data                \
		-j .dynamic             \
		-j .dynsym              \
		-j .rel                 \
		-j .rela                \
		-j .reloc               \
		--target=efi-app-x86_64 \
		$< $@

$(MAIN_SO): $(OBJS) $(CRT0_EFI)
	$(LD) $^ 			\
		-nostdlib		\
		-znocombreloc	\
		-T $(EFI_LDS)	\
		-shared			\
		-Bsymbolic		\
		-L $(LIB_PATH)	\
		-l:libgnuefi.a	\
		-l:libefi.a		\
		-o $@

.PHONY: clean
clean:
	$(RM) $(OBJS) $(MAIN_SO) $(TARGET)
