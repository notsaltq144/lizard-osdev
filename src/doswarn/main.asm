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
	mov $0x0e, %ah
loop:
	lodsb
	cmp $0x00, %al
	je end
	int $0x10
	jmp loop
end:
	mov $0x01,   %ah
	mov $0x0706, %cx
	int $0x10

halt:	jmp halt

string:  .asciz "firmware type error"

