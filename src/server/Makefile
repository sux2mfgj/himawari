TARGET := $(notdir $(shell pwd))

STARTUP	:= startup.o
#SERVERS	:= lib memory 
SERVERS	:= memory schedule
export LDFLAGS := -nostdlib -nostdinc

.PHONY: all
all: $(TARGET)

$(STARTUP): startup.S
	$(CC) $(LDFLAGS) -I../include -c -o $@ $<

.PHONY: $(TARGET)
$(TARGET): $(STARTUP)
	for dir in $(SERVERS); do\
		(cd $$dir; $(CARGO) build --release); \
	done
	for dir in $(SERVERS); do\
		$(LD) $(LDFLAGS) $(STARTUP) $$dir/target/release/lib$$dir.rlib -o $$dir.elf;\
	done

clean:
	rm -f $(OBJS) $(TARGET) *.o *.elf
	for dir in $(SERVERS) lib; do\
		(cd $$dir; $(CARGO) clean;)\
	done

