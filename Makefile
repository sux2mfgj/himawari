elf=build/himawari.elf

.PHONY: compile
compile: build $(elf)

build:
	meson setup build

.PHONY: reconf
reconf: build
	meson setup build --reconfigure

.PHONY: $(elf)
$(elf):
	meson compile -C build

.PHONY: run
run: compile
	./scripts/run-qemu.sh

.PHONY: run-d
run-d: compile
	./scripts/run-qemu.sh d


.PHONY: clean
clean:
	rm -rf build
