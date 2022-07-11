TARGET = BOOTX64.EFI
ALSO = img run
CFLAGS = -Wall -Wextra -Wundef -Wunused-macros -Wstrict-prototypes
USE_GCC = 1
SRCS = src/uefi.c
include uefi/Makefile
img:
	dd if=/dev/zero of=fat.img bs=1k count=1440
	mformat -i fat.img -T 524216 -F :: -v "lizardOS   "
	mmd -i fat.img ::/EFI
	mmd -i fat.img ::/EFI/BOOT
	mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT
	cp fat.img iso
	xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.img iso
run: img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom cdimage.img
gitignore:
	pastaignore -i .gitignore.pastaignore -o .gitignore --verbose --remove-duplicates

