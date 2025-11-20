#!/bin/bash

KERNEL="build/himawari.elf"

if [ ! -f "$KERNEL" ]; then
    echo "Error: $KERNEL not found. Please build first."
    exit 1
fi

echo "Starting QEMU with GDB server on port 1234..."
echo "Connect with: gdb $KERNEL -ex 'target remote :1234'"

stop_until_c=-S

args="-nographic -serial mon:stdio -s"

if [ $# -eq 1 ]; then
	args="$args -S"
fi

qemu-system-x86_64 \
    -kernel "$KERNEL" \
    -debugcon file:debugcon.txt \
    -d int,cpu_reset \
    -m 128M \
    ${args}

