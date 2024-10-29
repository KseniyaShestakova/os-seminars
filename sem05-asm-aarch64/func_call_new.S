	.arch armv8-a
	.file	"func_call.c"
	.text
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%ld"
	.text
	.align	2
	.global	get_int64
	.type	get_int64, %function
get_int64:
.LFB13:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	adrp	x0, :got:__stack_chk_guard
	mov	x29, sp
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x1, [x0]
	str	x1, [sp, 24]
	mov	x1, 0
	adrp	x0, .LC0
	add	x1, sp, 16
	add	x0, x0, :lo12:.LC0
	str	xzr, [sp, 16]
	bl	__isoc99_scanf
	adrp	x1, :got:__stack_chk_guard
	ldr	x0, [sp, 16]
	ldr	x1, [x1, #:got_lo12:__stack_chk_guard]
	ldr	x3, [sp, 24]
	ldr	x2, [x1]
	subs	x3, x3, x2
	mov	x2, 0
	beq	.L2
	bl	__stack_chk_fail
.L2:
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE13:
	.size	get_int64, .-get_int64
	.ident	"GCC: (Ubuntu 11.2.0-5ubuntu1) 11.2.0"
	.section	.note.GNU-stack,"",@progbits
