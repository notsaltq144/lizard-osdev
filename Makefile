include libMakefile/C.make
TARGET = BOOTX64.EFI
ALSO = img run
CFLAGS = $(C-GCC-WFLAGS)
USE_GCC = 1
SRCS = src/uefi.c
OUTDIR = int/
include uefi/Makefile

INTDIR = int/
img: doswarn
	dd if=/dev/zero of=fat.img bs=1k count=1440
	mformat -i fat.img -T 524216 -F :: -v "lizardOS   "
	mmd -i fat.img ::/EFI
	mmd -i fat.img ::/EFI/BOOT
	mcopy -i fat.img int/BOOTX64.EFI ::/EFI/BOOT
	cp fat.img iso
	xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.img iso
	dd if=$(OUTDIR)doswarn.bin of=cdimage.img bs=512 count=1 conv=notrunc
run: img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom cdimage.img
gitignore:
	pastaignore -i .gitignore.pastaignore -o .gitignore --verbose --remove-duplicates
doswarn:
	as src/doswarn.asm -o $(OUTDIR)doswarn.o
	ld -o $(OUTDIR)doswarn.bin $(OUTDIR)doswarn.o -e start --oformat binary -Ttext 0x7c00

