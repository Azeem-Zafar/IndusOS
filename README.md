# AzeemOS
First Pakistani OS built from scratch in Assembly + C

## Features
- Custom Bootloader
- 32-bit Protected Mode Kernel
- VGA Driver
- Keyboard Driver  
- File System
- Memory Manager

## Built by
Azeem Zafar — Pakistan 🇵🇰

## How to run
nasm -f bin boot/boot.asm -o boot/boot.bin
gcc -m32 -ffreestanding -fno-pie -c src/kernel.c -o src/kernel.o
ld -m elf_i386 -T src/linker.ld -o src/kernel.bin src/kernel.o --oformat binary
cat boot/boot.bin src/kernel.bin > os.img
qemu-system-x86_64 -fda os.img
