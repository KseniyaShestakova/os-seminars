	.file	"call_example.c"
	.intel_syntax noprefix
	.text
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%d"
.LC1:
	.string	"%d\n"
	.text
	.p2align 4
	.globl	cyclic_sum
	.type	cyclic_sum, @function
cyclic_sum:
.LFB23:
	.cfi_startproc
	endbr64
	push	r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	push	r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	push	r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	push	rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	push	rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	sub	rsp, 16
	.cfi_def_cfa_offset 64
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 8[rsp], rax
	xor	eax, eax
	test	rdi, rdi
	je	.L1
	mov	rbp, rdi
	xor	ebx, ebx
	lea	r14, 4[rsp]
	lea	r13, .LC0[rip]
	lea	r12, .LC1[rip]
	.p2align 4,,10
	.p2align 3
.L3:
	mov	rsi, r14
	mov	rdi, r13
	xor	eax, eax
	mov	DWORD PTR 4[rsp], 0
	call	__isoc99_scanf@PLT
	mov	rdx, QWORD PTR A[rip]
	movsx	rax, DWORD PTR 4[rsp]
	mov	rsi, r12
	mov	edi, 1
	movsx	rdx, DWORD PTR [rdx+rbx*4]
	add	rbx, 1
	imul	rdx, rax
	xor	eax, eax
	call	__printf_chk@PLT
	cmp	rbp, rbx
	jne	.L3
.L1:
	mov	rax, QWORD PTR 8[rsp]
	sub	rax, QWORD PTR fs:40
	jne	.L11
	add	rsp, 16
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	pop	rbx
	.cfi_def_cfa_offset 40
	pop	rbp
	.cfi_def_cfa_offset 32
	pop	r12
	.cfi_def_cfa_offset 24
	pop	r13
	.cfi_def_cfa_offset 16
	pop	r14
	.cfi_def_cfa_offset 8
	ret
.L11:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE23:
	.size	cyclic_sum, .-cyclic_sum
	.ident	"GCC: (Ubuntu 11.2.0-7ubuntu2) 11.2.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
