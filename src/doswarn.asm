.code16
.global start

start:
	ljmp $0, $stableSegment
stableSegment:
	mov $0x00,   %ax
	mov %ax,     %ss
	mov $0x7c00, %sp
	call videoMode
	call cursorPosition
	mov $string, %si
	call print
	call newline
	mov $string2, %si
	call print
	call cursorVisibility
	call halt
videoMode:
	mov $0x00, %ah
	mov $0x03, %al
	int $0x10
	ret
cursorPosition:
	mov $0x02, %ah
	mov $0x00, %bh
	mov $0x00, %dh
	mov $0x00, %dl
	int $0x10
	ret
cursorVisibility:
	mov $0x01, %ah
	mov $0x0706, %cx
	int $0x10
	ret
print:
	mov $0x0e, %ah
printLoop:
	mov (%si), %al
	cmp $0x00, %al
	je printEnd
	int $0x10
	inc %si
	jmp printLoop
printEnd:
	ret
newline:
	xor %bx, %bx
	mov $0x03, %ah
	int $0x10
	inc %dh
	xor %dl, %dl
	mov $0x02, %ah
	int $0x10
	ret
halt:
	cli
	hlt
	jmp halt
string:  .asciz "This disk is only bootable on UEFI devices."
string2: .asciz "Switch to UEFI mode or buy another computer."

.fill 510-(.-start), 1, 0
.word 0xaa55

