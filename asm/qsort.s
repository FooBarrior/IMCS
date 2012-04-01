#asm bubble-sort impl
.data

c_initial_state:
    .string "initial array state:"
fmt:
    .string " %lld\n"
c_sorted_array:
    .string "sorted array:\n"
    .equ c_sorted_array_len, . - c_sorted_array - 1
c_qsd:
    .string "\nEND OF DBO\n\n"
    .equ c_qsd_len, . - c_qsd - 1
min_fmt:
    .string "MIN: %llu\n"
.equ INT_SIZE, 8
.equ ARR_SIZE, 10
.equ MARS_A, 1 #Marsagilla 64-bit xorshift
.equ MARS_B, 1
.equ MARS_C, 55

array_size:
    .int ARR_SIZE*INT_SIZE
array:
    .space ARR_SIZE * INT_SIZE
array_end:

fast_itoa_buf:
    .space 21
fast_itoa_buf_end:
newline:
    .string "\n"

.globl _start

.text
qsort:
    movq INT_SIZE(%rsp), %rbx
    movq (2*INT_SIZE)(%rsp), %rcx
    cmpq %rbx, %rcx
    jbe qsort_return

qsort_start:
    movq %rbx, %rdi
    movq %rcx, %rsi

    movq (%rcx), %r8

qsort_step:
    cmpq %rdi, %rsi
    jbe qsort_rec

qsort_while_l:
    movq (%rdi), %rax
    cmpq %rax, %r8
    jae qsort_while_r
    addq $INT_SIZE, %rdi
    jmp qsort_while_l

qsort_while_r:
    movq (%rsi), %rdx
    cmpq %rdx, %r8
    jbe qsort_swap
    subq $INT_SIZE, %rsi
    jmp qsort_while_r

qsort_swap:
    cmpq %rdi, %rsi
    jb qsort_rec

    movq %rdx, (%rdi)
    movq %rax, (%rsi)
    subq $INT_SIZE, %rsi
    addq $INT_SIZE, %rdi

    jmp qsort_step

qsort_rec:
    pushq %rcx
    pushq %rdi
    pushq %rsi
    pushq %rbx
    call qsort
    call qsort
qsort_return:
    retq $(2*INT_SIZE)




/*
   fast_itoa: convert long int to ansi string with fast non-abi call interface
   for call use jmp
   input:
    %rbp - jump-back address
    %rax - input integer
   output:
    %rcx - result string buffer pointer
    %r15 = $fast_itoa_buf_end - static string buffer end pointer
   modified registers:
    %r15, %rax, %rcx, %rdx, %rflags
   notes:
    - if you dont use %r15, attempt to call very_fast_itoa with moving buffer end pointer to %r15 before
*/
fast_itoa:
    movq $fast_itoa_buf_end, %r15
very_fast_itoa:
    movq %r15, %rcx
    movq $10, %r11
fast_itoa_loop:
    xorl %edx, %edx
    divq %r11
    addl $0x30, %edx
    decl %ecx
    movb %dl, (%rcx)
    test %rax, %rax
    jne fast_itoa_loop
    jmp *%rbp


#ENTRY POINT
_start:
#16-byte frame alignment
    pushq %rbp

#inline randomize
    rdtsc
    shlq $32, %rdx
    orq %rdx, %rax
    movq %rax, %r13

    movq $array, %r12

#for-loop
    movl $ARR_SIZE, %ebx
    xorq %r14, %r14
    negq %r14
    movq $fast_itoa_buf_end, %r15 #optimize for very_fast_itoa call
loop:
#inline random
    movq %r13, %rax
    shlq $MARS_A, %rax
    xorq %rax, %r13
    movq %r13, %rax
    shrq $MARS_B, %rax
    xorq %rax, %r13
    movq %r13, %rax
    shlq $MARS_C, %rax
    xorq %rax, %r13
    
    movq %r13, %rax
#    shrq $0x30, %rax
    movq $write_0, %rbp
    jmp very_fast_itoa
write_0:
    movl $1, %eax
    movl %eax, %edi
    movl %ecx, %esi
    movq %r15, %rdx
    subl %ecx, %edx
    incl %edx
    syscall

    decl %ebx
    lea (%r12, %rbx, INT_SIZE), %rdx
    movq %r13, (%rdx)
#    shrq $0x30, (%rdx)
    test %ebx, %ebx
    jnz loop

    movl $1, %eax
    movl %eax, %edi
    movq $c_sorted_array, %rsi
    movl $c_sorted_array_len, %edx
    syscall

    leaq ((ARR_SIZE - 1) * INT_SIZE)(%r12), %rax
    pushq %rax
    pushq %r12
    call qsort
    
    movl $(ARR_SIZE - 1), %ebx
out_0:
    movq (%r12, %rbx, INT_SIZE), %rax
    movq $out_1, %rbp
    jmp very_fast_itoa
out_1:
    movl $1, %eax
    movl %eax, %edi
    movl %ecx, %esi
    movq %r15, %rdx
    subl %ecx, %edx
    incl %edx
    syscall
    decl %ebx
    jns out_0

out_2:

    popq %rbp
    movl $60, %eax
    xorl %edi, %edi
    syscall
