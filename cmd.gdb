target remote localhost:10000
file src/boot/main.so
add-symbol-file ./src/kernel/kernel.elf
