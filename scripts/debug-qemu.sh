#!/bin/bash

KERNEL="build/himawari.elf"

if [ ! -f "$KERNEL" ]; then
    echo "Error: $KERNEL not found. Please build first."
    exit 1
fi

echo "Starting GDB and connecting to QEMU on port 1234..."
echo "Make sure QEMU is running with GDB server enabled (-s -S flags)"
echo ""

gdb "$KERNEL" \
    -ex 'target remote :1234' \
    -ex 'layout asm' \
    -ex 'layout regs'
