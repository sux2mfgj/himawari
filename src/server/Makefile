TARGET := $(notdir $(shell pwd))

SERVERS	:= memory schedule
export LDFLAGS := -nostdlib -nostdinc

.PHONY: all
all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET):
	for dir in $(SERVERS); do\
		(cd $$dir; $(MAKE);)\
	done

clean:
	rm -f $(OBJS) $(TARGET) *.o
	for dir in $(SERVERS); do\
		(cd $$dir; $(MAKE) clean;)\
	done

