CC = gcc
ASM = nasm
LD = ld

CFLAGS = -Wall -Wextra -Wunreachable-code -Wcast-qual -Wcast-align -Wswitch-enum -Wmissing-noreturn -Wwrite-strings -Wundef -Wpacked -Wredundant-decls -Winline -Wdisabled-optimization -m32 -nostdinc -ffreestanding -fno-builtin
AFLAGS = -f elf
LFLAGS = -melf_i386 -nostdlib -T linker.ld

tetris.elf: entry.o tetris.o
	$(LD) $(LFLAGS) $^ -o $@

entry.o: entry.asm
	$(ASM) $(AFLAGS) $< -o $@

tetris.o: tetris.c
	$(CC) $(CFLAGS) $< -c -o $@

GENISOIMAGE = genisoimage
ISOHYBRID = isohybrid
ISOLINUXBIN = /usr/lib/syslinux/isolinux.bin
ISOLINUXMBOOTC32 = /usr/lib/syslinux/mboot.c32

tetris.iso: iso/boot/tetris.elf iso/boot/isolinux/isolinux.bin iso/boot/isolinux/mboot.c32 iso/boot/isolinux/isolinux.cfg
	$(GENISOIMAGE) -R -b boot/isolinux/isolinux.bin -no-emul-boot -boot-load-size 4 -boot-info-table -o $@ iso
	$(ISOHYBRID) $@

iso/boot/tetris.elf: tetris.elf
	@mkdir -p iso/boot
	cp $< $@

iso/boot/isolinux/isolinux.bin: $(ISOLINUXBIN)
	@mkdir -p iso/boot/isolinux
	cp $< $@

iso/boot/isolinux/mboot.c32: $(ISOLINUXMBOOTC32)
	@mkdir -p iso/boot/isolinux
	cp $< $@

iso/boot/isolinux/isolinux.cfg: isolinux.cfg
	@mkdir -p iso/boot/isolinux
	cp $< $@

QEMU = qemu-system-i386
QFLAGS = -soundhw pcspk

qemu: tetris.elf
	$(QEMU) $(QFLAGS) -kernel $<

qemu-iso: tetris.iso
	$(QEMU) $(QFLAGS) -cdrom $<

clean:
	rm -rf tetris.elf entry.o tetris.o iso

.PHONY: qemu qemu-iso clean
