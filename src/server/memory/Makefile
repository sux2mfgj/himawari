TARGET := $(notdir $(shell pwd)).elf

CRT_OBJ	:= ../../lib/c_run_time_startup.o
OBJS	:= $(CRT_OBJ) main.o
INCLUDE	:= -I../../include

.PHONY: all
all: $(TARGET)

.SUFFIXES: .o .c .S
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<
.S.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

$(TARGET): $(OBJS) Makefile
	$(LD) $(LDFLAGS) $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET) *.o
