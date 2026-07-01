#!/bin/bash

# Clean aur build
make clean
make

# kernel_main address nikalo
ADDR=$(objdump -d src/kernel.o | grep "^[0-9a-f]* <kernel_main>" | awk '{print $1}')
ADDR_DEC=$((16#$ADDR + 16#1000))
ADDR_HEX=$(printf "0x%X" $ADDR_DEC)

echo "kernel_main address: $ADDR_HEX"

# QEMU chalao seedha address pe
qemu-system-x86_64 -fda os.img -S -s &
sleep 1

# GDB se seedha jump karo
echo "Done - Running OS"
kill %1

# Normal run
qemu-system-x86_64 -fda os.img
