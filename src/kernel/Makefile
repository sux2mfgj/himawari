TARGET := kernel.o

OBJS	:= main.o

INCLUDE	:= -I../include

.PHONY: all
all: $(TARGET)

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@  $<

$(TARGET): $(OBJS) Makefile
	$(LD) $(LDFLAGS) -r $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET)
