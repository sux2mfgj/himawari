TARGET := $(notdir $(shell pwd))

STARTUP	:= startup.o
#SERVERS	:= lib memory 
SERVERS	:= memory process
export LDFLAGS := -nostdlib -nostdinc -static

.PHONY: all
all: $(TARGET)

$(STARTUP): startup.S
	$(CC) $(LDFLAGS) -I../include -c -o $@ $<

#          (cd $$dir; $(CARGO) build -p server -p $$dir --release); 
#
.PHONY: $(TARGET)
$(TARGET): $(STARTUP)
	for dir in $(SERVERS); do\
		(cd $$dir; $(CARGO) build -p server -p $$dir); \
	done
	for dir in $(SERVERS); do\
		$(LD) $(LDFLAGS) $(STARTUP) $$dir/target/debug/lib$$dir.a \
		-o $$dir.elf;\
	done

clean:
	rm -f $(OBJS) $(TARGET) *.o *.elf
	for dir in $(SERVERS) lib; do\
		(cd $$dir; $(CARGO) clean;)\
	done

