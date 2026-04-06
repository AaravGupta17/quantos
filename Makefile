all: os.bin

boot/boot.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o boot/boot.bin

kernel/kernel.o: kernel/kernel.c
	gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -c kernel/kernel.c -o kernel/kernel.o

kernel/memory.o: kernel/memory.c
	gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -c kernel/memory.c -o kernel/memory.o

kernel/rng.o: kernel/rng.c
	gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -c kernel/rng.c -o kernel/rng.o

kernel/kernel.bin: kernel/kernel.o kernel/memory.o kernel/rng.o kernel/linker.ld
	ld -m elf_i386 -T kernel/linker.ld -o kernel/kernel.bin kernel/kernel.o kernel/memory.o kernel/rng.o --oformat binary

os.bin: boot/boot.bin kernel/kernel.bin
	cat boot/boot.bin kernel/kernel.bin > os.bin

run: os.bin
	qemu-system-x86_64 -drive format=raw,file=os.bin

clean:
	rm -f boot/boot.bin kernel/kernel.o kernel/memory.o kernel/rng.o kernel/kernel.bin os.bin
