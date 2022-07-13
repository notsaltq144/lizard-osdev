include libMakefile/C.make
TARGET = BOOTX64.EFI
ALSO = img run
CFLAGS = $(C-GCC-WFLAGS)
USE_GCC = 1
SRCS = src/uefi.c
OUTDIR = int/
include uefi/Makefile

img: 
	dd if=/dev/zero of=hdimage.iso bs=1k count=1440
	./fdisk.sh
	mformat -i hdimage.iso -T 524216 -F :: -v "lizardOS   "
	mmd -i hdimage.iso ::/EFI
	mmd -i hdimage.iso ::/EFI/BOOT
	mcopy -i hdimage.iso int/BOOTX64.EFI ::/EFI/BOOT
run: img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -hda hdimage.iso
gitignore:
	pastaignore -i .gitignore.pastaignore -o .gitignore --verbose --remove-duplicates

