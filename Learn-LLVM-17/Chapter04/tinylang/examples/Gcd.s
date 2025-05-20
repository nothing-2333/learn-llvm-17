	.text
	.file	"Gcd.mod"
	.globl	_t3Gcd3GCD
	.p2align	4, 0x90
	.type	_t3Gcd3GCD,@function
_t3Gcd3GCD:
	.cfi_startproc
	movq	%rdi, %rax
	testq	%rsi, %rsi
	je	.LBB0_3
	.p2align	4, 0x90
	testq	%rsi, %rsi
	je	.LBB0_3
.LBB0_2:
	cqto
	idivq	%rsi
	movq	%rsi, %rax
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB0_2
.LBB0_3:
	retq
.Lfunc_end0:
	.size	_t3Gcd3GCD, .Lfunc_end0-_t3Gcd3GCD
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
