all: os.bin

boot/boot.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o boot/boot.bin

kernel/kernel.o: kernel/kernel.c
	gcc -m32 -ffreestanding -fno-pic -c kernel/kernel.c -o kernel/kernel.o

kernel/kernel.bin: kernel/kernel.o kernel/linker.ld
	ld -m elf_i386 -T kernel/linker.ld -o kernel/kernel.bin kernel/kernel.o --oformat binary

os.bin: boot/boot.bin kernel/kernel.bin
	cat boot/boot.bin kernel/kernel.bin > os.bin

run: os.bin
	qemu-system-x86_64 -drive format=raw,file=os.bin

clean:
	rm -f boot/boot.bin kernel/kernel.o kernel/kernel.bin os.bin
