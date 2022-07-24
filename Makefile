include libMakefile/C.make
TARGET = BOOTX64.EFI
ALSO = img run
CFLAGS = $(C-GCC-WFLAGS)
USE_GCC = 1
SRCS = src/uefi.c
OUTDIR = int/
include uefi/Makefile

EFIPARTNUM   = 1
EFIPARTSTART = 34
EFIPARTEND   = 2846

img: 
	dd if=/dev/zero of=hdimage.iso bs=1k count=1440
	./fdisk.sh $(EFIPARTNUM) $(EFIPARTSTART) $(EFIPARTEND)
	dd if=hdimage.iso of=int/efipart.iso skip=$(EFIPARTSTART) bs=512 count=$$(( ( $(EFIPARTEND) - $(EFIPARTSTART) ) + 1))
	mformat -i int/efipart.iso -T 524216 -F :: -v "lizardOS   "
	mmd -i int/efipart.iso ::/EFI
	mmd -i int/efipart.iso ::/EFI/BOOT
	mcopy -i int/efipart.iso int/BOOTX64.EFI ::/EFI/BOOT
	dd if=int/efipart.iso of=hdimage.iso bs=512 seek=34 conv=notrunc
run: img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -hda hdimage.iso
gitignore:
	pastaignore -i .gitignore.pastaignore -o .gitignore --verbose --remove-duplicates

