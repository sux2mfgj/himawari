TARGET	:= boot/kernel.elf

all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET): 
	cd kernel/; $(MAKE)
	cd boot/; $(MAKE)
	
.PHONY: clean
clean:
	cd kernel/; $(MAKE) clean
	cd boot/; $(MAKE) clean
