# AzeemOS 🇵🇰

Pakistan ka pehla custom OS — scratch se banaya gaya!

## Features
- GRUB Bootloader
- 32-bit Protected Mode Kernel
- VGA Driver
- Keyboard Driver
- File System (create/read/write/delete)
- Memory Manager (kmalloc)
- Interactive Shell

## How to Run
make
qemu-system-x86_64 -cdrom os.iso -m 64 -boot d -vga std

## Developer
Azeem Zafar — Pakistan
