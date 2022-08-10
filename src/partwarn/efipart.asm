BITS 16
ORG 0x7c5a
_start:
	xor ax, ax
	mov ax, ss
	mov sp, 0x7c00

	mov ds, ax

	mov ax, 0x0003
	int 0x10

	mov ah, 2
	mov bh, 2
	xor dx, dx
	int 0x10

	mov si, string
	call print

	xor bx, bx
	mov ah, 3
	int 0x10
	inc dh
	mov dl, 0
	mov ah, 2
	int 0x10

	mov si, string2
	call print
	mov ah, 1
	mov cx, 0x0706
	int 0x10

	halt: jmp halt

print:
	mov ah, 0x0e
.loop:
	lodsb
	cmp al, 0
	je .end
	int 0x10
	jmp .loop
.end:
	ret
string:  db "firmware type error", 0
string2: db "boot location error", 0
