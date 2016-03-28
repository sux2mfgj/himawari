TARGET	:= kernel.efi

MEMMAP	:= memory.map

#  LD_SCRIPT	:= kernel.ld
#  LDFLAGS	+= -Map $(MEMMAP) -T $(LD_SCRIPT)

INIT	:= main.o
INIT_OBJS	:= $(foreach file, $(INIT), init/$(file))

OBJS	:= $(INIT_OBJS)
MAIN_SO	:= main.so 

all: $(TARGET)

init/main.o: init/main.c Makefile
	$(CC) $< $(EFI_CFLAGS) $(EFI_INCLUDES) -c -o $@

#  .SUFFIXES: .c.o
#  %.o: %.c Makefile
#      $(CC) $(CFLAGS) -c -o $@ $<

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
