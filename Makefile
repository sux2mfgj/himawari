
OBJS	= kernel.o multiboot.o
IMAGE	= kernel 

INCLUDE_DIR = ./include

CC		= gcc
LD		= ld
RM		= rm
MAKE	= make
QEMU	= qemu-system-x86_64

CFLAGS 	= -Wall -g -I$(INCLUDE_DIR)
#  LDFLAGS	= -T linkerscript 
LDFLAGS	= -Ttext=0x100000 


.c.o:
	$(CC) -c $< -o $@

.S.o:
	$(CC) -c $< -o $@



default: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(IMAGE)


run:
	$(QEMU) -kernel $(IMAGE) 

clean:
	rm -f $(OBJS) $(IMAGE)
