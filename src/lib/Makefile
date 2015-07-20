CC := clang
AS := as
LD := ld
AR := ar

CFLAGS := -ggdb3 -Wall --target=i686-elf -I../include

LDFLAGS := -m elf_i386 --format elf32-i386 --oformat elf32-i386 \
		  -nostdlib -I../include 

LIBRARYS := libstring.a

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: all
all: $(LIBRARYS)

libstring.a: string.o
	ar r $@ string.o

clean:
	rm -f *.o 

