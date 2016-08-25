.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0:.string "argument 1 (a) is "
STR1:.string "argument 2 (b) is "
STR2:.string "(a^2-b^2)/-b is"
STR3:.string "\nReturning result into shell env. variable '$?'"
STR4:.string "'echo $?' displays last return value (as unsigned byte).\n"

.section .data
_x: .zero 8
_z: .zero 8
_y: .zero 8

.globl main
.section .text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $1, %rdi
	cmpq	$2,%rdi
	jne ABORT
	cmpq $0, %rdi
	jz SKIP_ARGS
	movq %rdi, %rcx
	addq $16, %rsi
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
	popq	%rsi
SKIP_ARGS:
	call	_f
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
_f:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%rdi
	pushq	%rsi
	movq -8(%rbp), _x
	movq -16(%rbp), _y
	movq $STR0, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $STR1, %rsi
	movq $strout, %rdi
	call printf
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $STR2, %rsi
	movq $strout, %rdi
	call printf
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	pushq %rax
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	cqo
	imulq (%rsp)
	addq $8, %rsp
	pushq %rax
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	pushq %rax
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	cqo
	imulq (%rsp)
	addq $8, %rsp
	pushq %rax
	movq 8(%rsp), %rax
	cqo
	idivq (%rsp)
	subq $8, %rsp
	addq $8, %rsp
	movq %rax, %rsi
	movq $intout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	pushq %rax
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	cqo
	imulq (%rsp)
	addq $8, %rsp
	pushq %rax
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	pushq %rax
	pushq _x
	movq _y, %rax
	addq (%rsp), %rax
	addq $8, %rsp
	cqo
	imulq (%rsp)
	addq $8, %rsp
	pushq %rax
	movq 8(%rsp), %rax
	cqo
	idivq (%rsp)
	subq $8, %rsp
	addq $8, %rsp
	movq %rax, _z
	movq $STR3, %rsi
	movq $strout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $STR4, %rsi
	movq $strout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $_z, %rax
	leave
	ret
