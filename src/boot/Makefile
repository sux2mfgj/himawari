ARCH	:= x86_64

CC		:= clang
LD		:= ld
OBJCOPY	:= objcopy

CFLAGS		:= -Wall
EFI_CFLAGS	:= -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPER $(CFLAGS)

LIB_PATH	:= /usr/lib
EFI_PATH:= /usr/include/efi
EFI_INCLUDES := -I $(EFI_PATH) -I $(EFI_PATH)/$(ARCH)
EFI_LDS		:= $(LIB_PATH)/elf_x86_64_efi.lds
CRT0_EFI	:= $(LIB_PATH)/crt0-efi-x86_64.o

TARGET 	:= BOOTX64.efi

C_OBJS	:= 
OBJS	:= main.o $(C_OBJS)
MAIN_SO	:= main.so 

.PHONY:all
all: $(TARGET)

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

main.o: main.c Makefile
	$(CC) $< $(EFI_CFLAGS) $(EFI_INCLUDES) -c -o $@

.PHONY:clean
clean:
	rm -f *.zip $(TARGET) $(OBJS) $(MAIN_SO)
