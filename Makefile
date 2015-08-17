# Binaries to use
TARGET = i386-elf
CC = $(TARGET)-gcc
ASM = nasm
LD = $(TARGET)-ld

# Compile and link flags
CWARNS = -Wall -Wextra -Wunreachable-code -Wcast-qual -Wcast-align -Wswitch-enum -Wmissing-noreturn -Wwrite-strings -Wundef -Wpacked -Wredundant-decls -Winline -Wdisabled-optimization
CFLAGS = -nostdinc -ffreestanding -fno-builtin -Os $(CWARNS)
AFLAGS = -f elf
LFLAGS = -nostdlib -T linker.ld

# Binary build

tetris.elf: entry.o tetris.o
	$(LD) $(LFLAGS) $^ -o $@

entry.o: entry.asm
	$(ASM) $(AFLAGS) $< -o $@

tetris.o: tetris.c config.h
	$(CC) $(CFLAGS) $< -c -o $@

# ISO build

GENISOIMAGE = genisoimage
GENISOFLAGS = -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table
STAGE2 = stage2_eltorito

tetris.iso: iso/boot/tetris.elf iso/boot/grub/stage2_eltorito iso/boot/grub/menu.lst
	$(GENISOIMAGE) $(GENISOFLAGS) -o $@ iso

iso/boot/tetris.elf: tetris.elf
	@mkdir -p iso/boot
	cp $< $@

iso/boot/grub/stage2_eltorito: $(STAGE2)
	@mkdir -p iso/boot/grub
	cp $< $@

iso/boot/grub/menu.lst: menu.lst
	@mkdir -p iso/boot/grub
	cp $< $@

# QEMU launchers

QEMU = qemu-system-i386
QFLAGS = -soundhw pcspk

qemu: tetris.elf
	$(QEMU) $(QFLAGS) -kernel $<

qemu-iso: tetris.iso
	$(QEMU) $(QFLAGS) -cdrom $<


clean:
	rm -rf tetris.elf entry.o tetris.o iso tetris.iso

.PHONY: qemu qemu-iso clean
