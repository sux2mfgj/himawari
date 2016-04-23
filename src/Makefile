KERNEL_PATH	:= boot

LINK_DIR := init lib kernel 
export LINK_OBJS = $(foreach dir, $(LINK_DIR), $(dir)/$(dir).o)

SERVER	:= server

all: $(KERNEL) $(SERVER)

.SUFFIXES: .o

.PHONY: $(KERNEL)
$(KERNEL): 
	for dir in $(LINK_DIR); do\
		(cd $$dir; $(MAKE);)\
	done
	cd boot/; $(MAKE)

.PHONY: $(SERVER)
$(SERVER):
	cd $@; $(MAKE)

.PHONY: clean
clean:
	for dir in $(LINK_DIR); do\
		(cd $$dir; $(MAKE) clean;)\
	done

	cd $(SERVER); $(MAKE) clean
	cd boot/; $(MAKE) clean
