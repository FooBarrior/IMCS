.equ N, 128
.equ INT_SIZE, 8
.equ DWORD_SIZE, 4

.globl main
.type main, @function
.align 32
.data
fmt_num:
    .string "%d "
fmt_2num:
    .string "%d%d"
.bss
n:
    .space DWORD_SIZE
m:
    .space DWORD_SIZE
adj:
    .space N * N * DWORD_SIZE
h:s0:
    .space DWORD_SIZE
s:
    .space DWORD_SIZE * N
maxh:maxs0:
    .space DWORD_SIZE
maxs:
    .space DWORD_SIZE * N

.text
main:
    pushq %rbp
#16-byte frame alignment
    subq $0x10, %rsp

    movq $fmt_num, %rdi
    movq $n, %rsi
    call scanf

    movq $fmt_num, %rdi
    movq $m, %rsi
    call scanf

    movl m, %ebx
    movq $N, %r12
    movq $1, %r13
    movq $adj, %r15
in_0:
        movq $fmt_2num, %rdi
        leaq (%rsp), %rsi
        leaq DWORD_SIZE(%rsp), %rdx
        call scanf
        xorq %rdi, %rdi
        xorq %rsi, %rsi
        movl (%rsp), %esi
        movl DWORD_SIZE(%rsp), %edi
        
        movl %r12d, %eax
        mull %esi
        movl %eax, %ecx
        
        movl %r12d, %eax
        mull %edi
        
        leaq (%r15, %rax, DWORD_SIZE), %rdx
        movl %r13d, (%rdx, %rsi, DWORD_SIZE)
        leaq (%r15, %rcx, DWORD_SIZE), %rdx
        movl %r13d, (%rdx, %rdi, DWORD_SIZE)
        decl %ebx
        jnz in_0

#movq $adj, %rdi
#movq $N, %rsi
#call outMatr
    movq $(N * DWORD_SIZE), %r12
    xorq %r9, %r9
    movq n, %r11      #%r11 <- n
    movq %r13, %rax   #%eax <- 1
                      #%r13 <- v
    xorq %rbx, %rbx   #%ebx <- h
    movq $s0, %r10    #%r10 <- s
    movq %r15, %r8    #%r8  <- adj[v]
    addq %r12, %r8    #adj[1]
    xorq %r14, %r14   #%r14 <- maxh
    movq $maxs0, %r15 #%r15 <- maxs
    cld
    call cliq
    
    orq %r14, %r14
    jz main_ret
out_0:
        movq $fmt_num, %rdi
        movl (%r15, %r14, DWORD_SIZE), %esi
        call printf
        decq %r14
        jnz out_0
main_ret:
    addq $0x10, %rsp
    popq %rbp
    ret

cliq:
    cmpl %r13d, %r11d
    jae cliq_step
    cmpq %rbx, %r14
    jae cliq_ret
    movl %ebx, %ecx
    leaq (%r15, %rax, DWORD_SIZE), %rdi
    leaq (%r10, %rax, DWORD_SIZE), %rsi
    rep movsd
    movq %rbx, %r14
    jmp cliq_ret

cliq_step:
       
    movl %ebx, %ecx  #%ecx <- i   #i   = h
#    movl %eax, %edx  #%edx <- can #can = 1
    orl %ecx, %ecx
    jz call1
cliq_for_1:
    movl (%r10, %rcx, DWORD_SIZE), %r9d #%r9 <- s[i]
    testl (%r8, %r9, DWORD_SIZE), %eax  #adj[v][s[i]]==1
    loopne cliq_for_1
    
#    testl %edx, %edx again?
    jz call2
call1:
    incl %ebx
    movl %r13d, (%r10, %rbx, DWORD_SIZE)
    incq %r13 #v++
    addq %r12, %r8
    call cliq
    decl %ebx
call2:
    incq %r13
    addq %r12, %r8
    call cliq

cliq_ret:
    decq %r13
    subq %r12, %r8
    ret


