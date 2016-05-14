TARGET := kernel.o

LINK_DIR := init
LINK_OBJS	:= $(foreach dir, $(LINK_DIR), $(dir)/$(dir).o)

OBJS	:= main.o early_memory.o pagetable.o vram_text.o trap.o entry.o irq.o process.o syscall.o schedule.o system.o task_call.o idle.o 

INCLUDE	:= -I../include -I../server/include

.PHONY: all
all: $(TARGET)

.SUFFIXES: .o .c .S
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<
.S.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

$(LINK_OBJS):
	for dir in $(LINK_DIR); do \
		(cd $$dir; $(MAKE);)\
	done

$(TARGET): $(OBJS) $(LINK_OBJS) Makefile

	$(LD) $(LDFLAGS) -r $(OBJS) $(LINK_OBJS) -o $@ 

clean:
	for dir in $(LINK_DIR); do\
		(cd $$dir; $(MAKE) clean;)\
	done

	rm -f $(OBJS) $(TARGET) *.o
