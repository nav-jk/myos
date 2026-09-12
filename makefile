# Automatically generate lists of sources using wildcards.
C_SOURCES = $(wildcard kernel/*.c drivers/*.c io/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h io/*.h)

# Convert the *.c filenames to *.o.
OBJ = ${C_SOURCES:.c=.o}

# Default build target
all: os-image

run: all
	bochs

run-qemu: all
	qemu-system-i386 -drive format=raw,file=os-image

# Build the disk image.
os-image: boot/boot_sect.bin kernel.bin
	cat $^ > os-image
	truncate -s 32K os-image

# Build the kernel.
kernel.bin: kernel/kernel_entry.o ${OBJ}
	ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

# Compile C files.
%.o: %.c ${HEADERS}
	gcc -ffreestanding -m32 -fno-pic -fno-pie -c $< -o $@

# Assemble kernel entry.
%.o: %.asm
	nasm $< -f elf -o $@

# Assemble boot sectors.
%.bin: %.asm
	nasm $< -f bin -I '../../16bit/' -o $@

clean:
	rm -fr *.bin *.dis *.o os-image
	rm -fr kernel/*.o drivers/*.o io/*.o boot/*.bin