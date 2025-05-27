	.text
	.file	"Gcd.mod"
	.globl	_t3Gcd3GCD
	.p2align	4, 0x90
	.type	_t3Gcd3GCD,@function
_t3Gcd3GCD:
	.cfi_startproc
	movq	%rdi, %rax
	testq	%rsi, %rsi
	jne	.LBB0_1
	retq
	.p2align	4, 0x90
.LBB0_2:
	cqto
	idivq	%rsi
	movq	%rsi, %rax
	movq	%rdx, %rsi
.LBB0_1:
	testq	%rsi, %rsi
	jne	.LBB0_2
	movq	%rax, .L_t3Gcd1p(%rip)
	retq
.Lfunc_end0:
	.size	_t3Gcd3GCD, .Lfunc_end0-_t3Gcd3GCD
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
