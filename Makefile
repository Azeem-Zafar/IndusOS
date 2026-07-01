all: os.iso

src/multiboot.o:
	nasm -f elf32 src/multiboot.asm -o src/multiboot.o

src/kernel.o:
	gcc -m32 -ffreestanding -fno-pie -c src/kernel.c -o src/kernel.o

src/kernel.bin: src/multiboot.o src/kernel.o
	ld -m elf_i386 -T src/linker.ld -o src/kernel.bin src/multiboot.o src/kernel.o

iso/boot/kernel.bin: src/kernel.bin
	cp src/kernel.bin iso/boot/kernel.bin

os.iso: iso/boot/kernel.bin
	grub-mkrescue -o os.iso iso

run: os.iso
	qemu-system-x86_64 -cdrom os.iso

clean:
	rm -f src/multiboot.o src/kernel.o src/kernel.bin iso/boot/kernel.bin os.iso
