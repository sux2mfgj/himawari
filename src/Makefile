TARGET	:= boot/kernel.elf

all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET): 
	cd init/; $(MAKE)
	cd lib/; $(MAKE)
	cd boot/; $(MAKE)
	
.PHONY: clean
clean:
	cd init/; $(MAKE) clean
	cd lib/; $(MAKE) clean
	cd boot/; $(MAKE) clean
