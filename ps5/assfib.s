.globl          main

.section        .data
fibstr:         .string "f(50) = %lu\n"

.section        .text
main:
        pushq   %rbp
        movq    %rsp, %rbp
        movq    $0, %rax
        movq    $1, %rsi
        movq    $25, %rcx
        
looptop:
        addq    %rsi, %rax
        addq    %rax, %rsi
        decq    %rcx
        cmp     $0, %rcx
        jne     looptop
        
        movq    %rax, %rsi
        movq    $fibstr, %rdi
        call    printf
        leave
        ret

