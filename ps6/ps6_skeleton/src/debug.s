.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0: .string "The"
STR1: .string "th fibonacci number is"
.section .data
.globl main
.section .text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $1, %rdi
	cmpq	$1,%rdi
	jne ABORT
	cmpq $0, %rdi
	jz SKIP_ARGS
	movq %rdi, %rcx
	addq $8, %rsi
PARSE_ARGV:
	pushq %rcx
	pushq %rsi
	movq (%rsi), %rdi
	movq $0, %rsi
	movq $10, %rdx
	call strtol
	popq %rsi
	popq %rcx
	pushq %rax
	subq $8, %rsi
	loop PARSE_ARGV
	popq	%rdi
SKIP_ARGS:
	call	_fibonacci_recursive
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
_fibonacci_recursive:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rdi
	subq	$8, %rsp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%r8
	pushq	%r9
	movq	-8(%rbp), %rax
	pushq	%rax
	popq	%rdi
	call	_fibonacci_number
	popq	%r9
	popq	%r8
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	movq	%rax, -8(%rbp)
	movq	$STR0, %rsi
	movq	$strout, %rdi
	call	printf
	movq	-8(%rbp), %rsi
	movq	$intout, %rdi
	call	printf
	movq	$STR1, %rsi
	movq	$strout, %rdi
	call	printf
	movq	-8(%rbp), %rsi
	movq	$intout, %rdi
	call	printf
	movq	$'\n', %rdi
	call	putchar
	movq	$0, %rax
	leave
	ret
_fibonacci_number:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rdi
	subq	$8, %rsp
	movq	$1, %rax
	movq	%rax, -8(%rbp)
	pushq	%rdx
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	$0, %rax
	popq	%rdx
	cmpq	%rax, %rdx
	popq	%rdx
	jle	ELSE1
	pushq	%rdx
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	$1, %rax
	subq	%rax, (%rsp)
	popq	%rax
	pushq	%rax
	movq	$0, %rax
	popq	%rdx
	cmpq	%rax, %rdx
	popq	%rdx
	jle	ENDIF2
	pushq	%rdx
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	$2, %rax
	subq	%rax, (%rsp)
	popq	%rax
	pushq	%rax
	movq	$0, %rax
	popq	%rdx
	cmpq	%rax, %rdx
	popq	%rdx
	jle	ENDIF3
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%r8
	pushq	%r9
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	$1, %rax
	subq	%rax, (%rsp)
	popq	%rax
	pushq	%rax
	popq	%rdi
	call	_fibonacci_number
	popq	%r9
	popq	%r8
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	pushq	%rax
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%r8
	pushq	%r9
	movq	-8(%rbp), %rax
	pushq	%rax
	movq	$2, %rax
	subq	%rax, (%rsp)
	popq	%rax
	pushq	%rax
	popq	%rdi
	call	_fibonacci_number
	popq	%r9
	popq	%r8
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	addq	%rax, (%rsp)
	popq	%rax
	movq	%rax, -8(%rbp)
ENDIF3:
ENDIF2:
	jmp	ENDIF1
ELSE1:
	movq	$0, %rax
	movq	%rax, -8(%rbp)
ENDIF1:
	movq	-8(%rbp), %rax
	leave
	ret
