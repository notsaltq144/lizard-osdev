PRINTDIRECTORY        = --no-print-directory
BOOTLOADER-PARTFILE   = int/parts/boot.prt
BOOTLOADER-OBJECTFILE = int/boot.o
BOOTLOADER-SOURCEFILE = src/boot.s
KERNEL-PARTFILE       = int/parts/detailed-boot.prt
KERNEL-OBJECTFILE     = int/detailed-boot.o
KERNEL-SOURCEFILE     = src/detailed-boot.c
GCC                   = ~/opt/cross/bin/i686-elf-gcc
LD                    = ~/opt/cross/bin/i686-elf-ld
VM                    = qemu-system-x86_64
SYSFILE               = lizard.bin

full:
	make bootloader $(PRINTDIRECTORY)
	make kernel $(PRINTDIRECTORY)
	truncate -s 32768 ./int/parts/detailed-boot.prt
	make join $(PRINTDIRECTORY)
bootloader:
	as -o $(BOOTLOADER-OBJECTFILE) $(BOOTLOADER-SOURCEFILE)
	ld -o $(BOOTLOADER-PARTFILE) --oformat binary -e init $(BOOTLOADER-OBJECTFILE) -Ttext 0x7c00
kernel:
	$(GCC) -ffunction-sections -ffreestanding $(KERNEL-SOURCEFILE) -o $(KERNEL-OBJECTFILE) -nostdlib -Wall -Wextra -O0
	$(LD) -o $(KERNEL-PARTFILE) -Ttext 0x7e00 --oformat binary $(KERNEL-OBJECTFILE) -e main --script=LDfile -O 0 -Ttext-segment 0x7e00 --verbose
join:
	cat $(BOOTLOADER-PARTFILE) $(KERNEL-PARTFILE) > $(SYSFILE)
run:
	$(VM) $(SYSFILE)
debug:
	$(VM) $(SYSFILE) -gdb tcp:localhost:6000 -S

