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
    -cpu max \
    -M q35 \
    -smp 2 \
    -netdev tap,ifname=tap0,id=net0,script=no,downscript=no \
    -device virtio-net-pci,netdev=net0,packed=on \
    -d int,cpu_reset,guest_errors \
    -D qemu_debug.log \
    -m 128M \
    ${args}




#    -device virtio-net-pci,netdev=net0,packed=on,disable-modern=off,disable-legacy=on \
