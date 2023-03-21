BITS 64
global call_function_with_gregs

%include "src/regs.asmh"

; Call function with general registers
; Calls a given function with given general registers (excluding rsp)
; call_function_with_gregs(void *func, gregs_t *regs) __attribute__((sysv_abi, no_caller_saved_registers));

call_function_with_gregs:
; don't crash in case of CET
	endbr64
; preserve registers
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	push rdi     ; save func
	mov r15, rsi ; use r15 as regs

; set registers
	mov rax, [r15+rrax]
	mov rbx, [r15+rrbx]
	mov rcx, [r15+rrcx]
	mov rdx, [r15+rrdx]
	mov rbp, [r15+rrbp]
	mov rsi, [r15+rrsi]
	mov rdi, [r15+rrdi]
	mov r8,  [r15+rr8]
	mov r9,  [r15+rr9]
	mov r10, [r15+rr10]
	mov r11, [r15+rr11]
	mov r12, [r15+rr12]
	mov r13, [r15+rr13]
	mov r14, [r15+rr14]
	mov r15, [r15+rr15]
	
; call function
	call [rsp]
	pop qword [rsp]
	
; restore registers
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
