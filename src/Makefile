TARGET	:= boot/kernel.elf

all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET): 
	cd init/; $(MAKE)
	cd boot/; $(MAKE)
	
.PHONY: clean
clean:
	cd init/; $(MAKE) clean
	cd boot/; $(MAKE) clean
