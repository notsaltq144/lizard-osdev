.code16         # 16 bit mode
.global init    # make label init global

init:
	call enableA20
reset:

 	mov $0x00, %ah # 0 = reset drive
 	mov $0x80, %dl # boot disk
 	int $0x13
 	jc reset
load:
	mov $0x42, %ah                        # 42 = extended read

	mov $0x8000,             %si
	xor %bx,                 %bx

	movl $0x00007e00,         %ds:4 (%si,1)
	movl $0x00400010,         %ds:0 (%si,1)
	mov  %cs,                 %ds:6 (%si,1)
	movl $0x00000001,         %ds:8 (%si,1) # start sector in lba
	movl $0x00000000,         %ds:12(%si,1) # start sector in lba
	int  $0x13

	# 1. Disable interrupts
	cli
	# 2. Load GDT
	lgdt (gdt_descriptor)
	# set 32 bit mode
	mov %cr0, %eax
	or  $1,   %eax
	mov %eax, %cr0
	# Far jmp
	jmp %cs:(code32)

checkA20:
	push %ds

	xor %ax, %ax
	mov %ax, %ds

	movw $0xAA55, %ax
	movw $0x7DFE, %bx
	movw (%bx), %bx
	cmpw %ax, %bx
	jnz checkA20_enabled
checkA20_disabled:
	xor %ax, %ax
	jmp checkA20_done
checkA20_enabled:
	xor %ax, %ax
	inc %ax
checkA20_done:
	pop %ds
	ret


enableA20:
	call checkA20
	jnz enableA20_enabled

enableA20_int15:
	mov $0x2403, %ax                 # A20 gate support
	int $0x15
	jb enableA20_keyboardController  # INT 15 aint supported
	cmp $0, %ah
	jnz enableA20_keyboardController # INT 15 aint supported

	mov $0x2402, %ax                 # A20 status
	int $0x15
	jb enableA20_keyboardController  # couldnt get status
	cmp $0, %ah
	jnz enableA20_keyboardController # couldnt get status

	cmp $1, %al
	jz enableA20_enabled             # A20 is activated

	mov $0x2401, %ax                 # A20 activation
	int $0x15
	jb enableA20_keyboardController  # couldnt activate
	cmp $0, %ah
	jnz enableA20_keyboardController # couldnt activate

enableA20_keyboardController:
	call checkA20
	jnz enableA20_enabled

	cli

	call enableA20_wait
	mov $0xAD, %al
	out %al,   $0x64

	call enableA20_wait
	mov $0xD0, %al
	out %al,   $64

	call enableA20_wait2
	in  $0x60, %al
	push %eax

	call enableA20_wait
	mov $0xD1, %al
	out %al,   $0x64

	call enableA20_wait
	pop %eax
	or  $2, %al
	out %al, $0x60

	call enableA20_wait
	mov $0xAE, %al
	out %al,   $0x64

	call enableA20_wait
	sti

enableA20_fastA20:
	call checkA20
	jnz enableA20_enabled

	in $0x92, %al
	test $2,  %al
	jnz enableA20_postFastA20
	or  $2,    %al
	and $0xFE, %al
	out %al,   $92

enableA20_postFastA20:
	call checkA20
	jnz enableA20_enabled
	cli
	hlt
enableA20_enabled:
	ret
enableA20_wait:
	in   $0x64, %al
	test $2,    %al
	jnz enableA20_wait
	ret
enableA20_wait2:
	in   $0x64, %al
	test $1,    %al
	jnz enableA20_wait2
	ret
setGDT: ret
# NOTE limit is the length
# NOTE base is the start
# NOTE base + limit = last address
gdt_start:
gdt_null:
# null descriptor
	.quad 0
gdt_data:
	.word 0x01c8 # limit: bits 0-15
	.word 0x0000 # base:  bits 0-15
	.byte 0x00   # base:  bits 16-23
# segment presence: yes (+0x80)
# descriptor priviledge level: ring 0 (+0x00)
# descriptor type: code/data (+0x10)
# executable: no (+0x00)
# direction bit: grows up (+0x00)
# writable bit: writable (+0x02)
# accesed bit [best left 0, cpu will deal with it]: no (+0x00)
	.byte 0x80 + 0x10 + 0x02
# granularity flag: limit scaled by 4kib (+0x80)
# size flag: 32 bit pm (+0x40)
# long mode flag: 32pm/16pm/data (+0x00)
# reserved: reserved (+0x00)
	.byte 0x80 + 0x40 # flags: granularity @ 4-7 limit: bits 16-19 @ 0-3
	.byte 0x00 # base:  bits 24-31
gdt_code:
	.word 0x0100 # limit: bits 0-15
	.word 0x8000 # base:  bits 0-15
	.byte 0x1c   # base:  bits 16-23   
# segment presence: yes (+0x80)
# descriptor priviledge level: ring 0 (+0x00)
# descriptor type: code/data (+0x10)
# executable: yes (+0x08)
# conforming bit [0: only ring 0 can execute this]: no (+0x00)
# readable bit: yes (0x02)
# accessed bit [best left 0, cpu will deal with it]: no (0x00)
	.byte 0x80 + 0x10 + 0x08 + 0x02
# granularity flag: limit scaled by 4kib (+0x80)
# size flag: 32 bit pm (+0x40)
# long mode flag: 32pm/16pm/data (+0x00)
# reserved: reserved (+0x00)
	.byte 0x80 + 0x40 + 0x00   # flags: granularity @ 4-7 limit: bits 16-19 @ 0-3
	.byte 0x00                 # base:  bits 24-31
gdt_end:
gdt_descriptor:
	.word gdt_end - gdt_start - 1
	.long gdt_start

.code32
code32:
	mov %ds, %ax
	mov %ax, %ds
#	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	movl $0x4000,  %ebp
	mov  %esp,     %ebp

	push $0x7e00
	ret
.fill 500-(.-init)
.quad 1
.word 1

.word 0xaa55
kernel:

