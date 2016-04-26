TARGET := lib.o

OBJS	:= string.o

INCLUDE	:= -I../include

.PHONY: all
all: $(TARGET) c_run_time_startup.o

.SUFFIXES: .o .c .S
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@  $<
.S.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@  $<


$(TARGET): $(OBJS) Makefile
	$(LD) $(LDFLAGS) -r $(OBJS) -o $@ 

clean:
	rm -f $(OBJS) $(TARGET) *.o
