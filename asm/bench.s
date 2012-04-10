.globl main
.type main, @function
.equ N, 10000000

.data
fmt_int: .string "%s\t# %llu\n"
.text
.macro _SYNC_
    xorq %rax, %rax
    cpuid
.endm
.macro _RDTSC_ reg
    rdtsc
    movq %rdx, %\reg
    shlq $32, %\reg
    orq %rax, %\reg
.endm

.macro TST s=\0
    jmp 3f
2: .string "\s"
3:
#    xorq %rax, %rax
#    cpuid
    _SYNC_
    _RDTSC_ rdi
    movq %rdi, %rbx #for application usage
    movq $N, %rcx
1:
.endm

.macro ENDTEST
    loop 1b
    _SYNC_
    _RDTSC_ rcx
    subq %rdi, %rcx
.endm
.macro _CALC_
    movq %rcx, %r15
    
    movq %rcx, %rax
    subq %r14, %rax
    movq $N, %rbx
    xorq %rdx, %rdx
    divq %rbx
    movq %rax, %rcx
.endm
.macro _PRINT_
    movq $fmt_int, %rdi
    movq %rcx, %rdx
    movq $2b, %rsi
    call printf
    mov $100000, %rbx
    call sched_yield
.endm
.macro ENDTESTP
    ENDTEST
    _CALC_
    _PRINT_
.endm

.macro ENDTESTN
    ENDTEST
.endm

.macro TSTLN line, p=P
    TST "\line"
    \line
    ENDTEST\p
.endm

.equ EMPTY_CHK_CNT, 1
main:
    pushq %rbp
    _SYNC_
    _SYNC_
    _SYNC_
#    _RDTSC_ rbx
#    _RDTSC_ rbx
#    _RDTSC_ rbx

    movq $EMPTY_CHK_CNT, %r12
    movq $123456789012345, %r14
empty_chk_loop:
    TST
    ENDTEST
    cmpq %rcx, %r14
    cmovaq %rcx, %r14
    dec %r12
    jnz empty_chk_loop
    movq %r14, %rcx
    _PRINT_
    TST nop
    nop
    ENDTESTP

.macro TSTMOV p, v, reg
    TSTLN "mov\p $\v, %\reg"
.endm
.macro TSTMOVXB p, v, reg
    TST "xor\p %\reg, %\reg; mov\p $\v, %\reg"
    xor\p %\reg, %\reg
    mov\p $\v, %\reg
    ENDTESTP
.endm
.macro TSTMOVu p, v, reg
    TSTMOV \p, \v, \reg
    TSTMOVXB \p, \v, \reg
.endm

    TSTMOVu q, -1, rax
    TSTMOVu l, -1, eax
    TSTMOVu w, -1, ax
    TSTMOVu b, -1, al
    TSTMOVu b, -1, ah

    TST "movq $0xabc, %rax; addq $0xdef, %rax"
    movq $0xabc, %rax
    addq $0xdef, %rax
    ENDTESTP

    pushq $1
    movq %rsp, %rbp
    pushq $0
    TST "move from memory, into memory, each time changing the value"
    movq (%rbp), %rax
    neg %rax
    movq %rax, (%rbp)
    ENDTESTP

    TST "32bit marsaglia xorshift"
    movl %ebx, %eax
    shll $1, %eax
    xorl %eax, %ebx
    movl %ebx, %eax
    shrl $3, %eax
    xorl %eax, %ebx
    movl %ebx, %eax
    shll $10, %eax
    xorl %eax, %ebx
    movl %ebx, %eax
#    shlq $32, %rax
#    shrq $38, %rax
#    addl $(1 << 25), %eax
#    movq (%eax), %rdx
    ENDTESTP

    TST "64bit xorshift"
    movq %rbx, %rax
    shlq $7, %rax
    xorq %rax, %rbx
    movq %rbx, %rax
    shrq $19, %rax
    xorq %rax, %rbx
    movq %rbx, %rax
    shlq $54, %rax
    xorq %rax, %rbx
    movq %rbx, %rax 
    ENDTESTP

    TST "DIV long"
    xorq %rdx, %rdx
    movq $2135461, %rax
    movq $123, %rsi
    divl %esi
    ENDTESTP

    TST "DIV quad big on small"
    xorq %rdx, %rdx
    movq $0x213456789ACBD, %rax
    movq $3, %rsi
    divq %rsi
    ENDTESTP

    TST "DIV quad big on big"
    xorq %rdx, %rdx
    movq $0x213456789ACBD, %rax
    movq $34512321213, %rsi
    divq %rsi
    ENDTESTP

    TST "DIV quad big on big with rdx"
    movq $0x54BAD234BEEFDEAD, %rdx
    movq $0x213456789ACBDEDD, %rax
    movq $0xCCC23ABBADDA3111, %rsi
    divq %rsi
    ENDTESTP

    TST "MUL"
    movq $0xA567D389A567D389, %rax
    movq $0x7523D45544554455, %rsi
    mulq %rsi
    ENDTESTP

    TST "MUL byte"
    movb $0xA5, %al
    movb $0xDE, %ah
    mulb %ah
    ENDTESTP
    popq %rax
    popq %rax
    popq %rbp
    xorq %rax, %rax
    ret
