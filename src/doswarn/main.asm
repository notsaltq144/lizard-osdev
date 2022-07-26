.code16
.global start

start:
	ljmp $0, $stableSegment
stableSegment:
	xor %ax,     %ax
	mov %ax,     %ss
	mov $0x7c00, %sp

	xor %ax, %ax
	mov %ax, %ds

	mov $0x0003, %ax
	int $0x10

	mov $0x02, %ah
	mov $0x00, %bh
	xor %dx, %dx
	int $0x10

	mov $string, %si
	call print

	xor %bx,   %bx
	mov $0x03, %ah
	int $0x10
	inc %dh
	mov $0,    %dl
	mov $0x02, %ah
	int $0x10

	mov $string2, %si
	call print

	mov $0x01,   %ah
	mov $0x0706, %cx
	int $0x10

halt:	jmp halt

print:
	mov $0x0e, %ah
printLoop:
	lodsb
	cmp $0x00, %al
	je printEnd
	int $0x10
	jmp printLoop
printEnd:
	ret
string:  .asciz "This disk is only bootable on UEFI devices."
string2: .asciz "Switch to UEFI mode or buy another computer."

