include libMakefile/C.make
TARGET = BOOTX64.EFI
ALSO = img run
CFLAGS = $(C-GCC-WFLAGS) -DUEFI_NO_TRACK_ALLOC
USE_GCC = 1
SRCS = src/uefi.c
OUTDIR = int
EXTRA = int/call_function_with_regs.o int/crc64.o
include uefi/Makefile

EFIPARTNUM   = 1
EFIPARTSTART = 34
EFIPARTEND   = 2846

img: doswarn file partwarn
	dd if=/dev/zero of=hdimage.iso bs=1k count=1440
	./fdisk.sh $(EFIPARTNUM) $(EFIPARTSTART) $(EFIPARTEND)
	dd if=hdimage.iso of=int/efipart.iso skip=$(EFIPARTSTART) bs=512 count=$$(( ( $(EFIPARTEND) - $(EFIPARTSTART) ) + 1))
	mformat -i int/efipart.iso -T 524216 -F :: -v "lizardOS   "
	mmd -i int/efipart.iso ::/EFI
	mmd -i int/efipart.iso ::/EFI/BOOT
	mcopy -i int/efipart.iso int/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i int/efipart.iso int/file.bin ::/
	dd if=int/efipartwarn.bin of=int/efipart.iso bs=1 seek=90 conv=notrunc
	dd if=int/efipart.iso of=hdimage.iso bs=512 seek=34 conv=notrunc
	dd if=int/dosmain.bin of=hdimage.iso conv=notrunc
	dd if=int/dossign.bin of=hdimage.iso bs=1 seek=510 conv=notrunc
run: img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -hda hdimage.iso
doswarn: int/dosmain.bin int/dossign.bin
partwarn: int/efipartwarn.bin
int/dosmain.bin: src/doswarn/main.asm
	as src/doswarn/main.asm -o $(OUTDIR)dosmain.o
	ld -o $(OUTDIR)dosmain.bin $(OUTDIR)dosmain.o -e start --oformat binary -Ttext 0x7c00
int/dossign.bin: src/doswarn/sign.asm
	as src/doswarn/sign.asm -o $(OUTDIR)dossign.o
	ld -o $(OUTDIR)dossign.bin $(OUTDIR)dossign.o -e _start --oformat binary
int/call_function_with_regs.o: src/call_function_with_regs.asm
	nasm -felf64 -o int/call_function_with_regs.o src/call_function_with_regs.asm
int/efipartwarn.bin: src/partwarn/efipart.asm
	nasm -fbin src/partwarn/efipart.asm -o int/efipartwarn.bin
int/crc64.o: src/crc64/crc64.c
	gcc -Ofast src/crc64/crc64.c -o int/crc64.o -I./uefi -ffreestanding -c
file:
	nasm -fbin src/file.asm -o int/file.bin

