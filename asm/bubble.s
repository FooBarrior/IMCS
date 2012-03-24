#asm bubble-sort impl
.data

c_initial_state:
    .string "initial array state:"
fmt:
    .string " %lld\n"
c_sorted_array:
    .string "sorted array:\n"
    .equ c_sorted_array_len, . - c_sorted_array - 1
min_fmt:
    .string "MIN: %llu\n"
.equ INT_SIZE, 8
.equ ARR_SIZE, 40
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

.globl main
.type   main, @function 

.text
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
main:
#16-byte alignment
    pushq %rbp

#inline randomize
    rdtsc
    shlq $32, %rdx
    orq %rdx, %rax
    movq %rax, %r13

    movq $array, %r12

#for-loop
    movl $ARR_SIZE, %ebx
    movq $-1, %r14
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
    
#some finding minimum test stuff
/*
    cmpq %r14, %r13
    ja loop_f
    movq %r13, %r14
    movq $min_fmt, %rdi
    movq %r13, %rsi
    call printRR_SIZE, %ebx                                                                                               
    f
loop_f:
    cmpq $2000000000, %r13
    ja loop
*/

    movq %r13, %rax
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
    jnz loop

    movl $1, %eax
    movl %eax, %edi
    movq $c_sorted_array, %rsi
    movl $c_sorted_array_len, %edx
    syscall

    movl $(ARR_SIZE + 1), %eax
bubble_0:
    decl %eax
    jz bubble_end
    movl $(ARR_SIZE - 2), %ebx
    movl $(ARR_SIZE - 1), %edx
bubble_1:
    movq (%r12, %rbx, INT_SIZE), %rdi
    movq (%r12, %rdx, INT_SIZE), %rsi
    cmpq %rdi, %rsi
    ja bubble_2
    jmp bubble_3
bubble_2:
    movq %rdi, (%r12, %rdx, INT_SIZE)
    movq %rsi, (%r12, %rbx, INT_SIZE)
bubble_3:
    decl %ebx
    decl %edx
    jz bubble_0
    jmp bubble_1
bubble_end:
    
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
    mov $0, %eax
    ret

