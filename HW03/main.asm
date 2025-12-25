bits 64

extern  _malloc, _puts, _printf, _fflush, _abort
global  _main

section .data
empty_str:   db 0
int_format:  db "%ld ", 0
data:        dq 4, 8, 15, 16, 23, 42
data_length: equ ($ - data) / 8

section .text

;;; print_int(long)
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 24                ; stack alignment

    mov rsi, rdi
    lea rdi, [rel int_format]
    xor rax, rax
    call _printf

    xor rdi, rdi
    call _fflush

    mov rsp, rbp
    pop rbp
    ret

;;; p(long) -> long
p:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element(value, next)
add_element:
    push rbp
    push rbx
    push r14
    mov rbp, rsp
    sub rsp, 24

    mov r14, rdi
    mov rbx, rsi

    mov rdi, 16
    call _malloc
    test rax, rax
    jz _abort

    mov [rax], r14
    mov [rax + 8], rbx

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m(list, fn)
m:
    push rbp
    mov rbp, rsp
    sub rsp, 24

    test rdi, rdi
    jz .out

    push rbx
    push r12

    mov rbx, rdi        ; list
    mov r12, rsi        ; fn

    mov rdi, [rbx]
    call r12

    mov rdi, [rbx + 8]
    mov rsi, r12
    call m

    pop r12
    pop rbx

.out:
    mov rsp, rbp
    pop rbp
    ret


;;; f(list, acc, pred)
f:
    push rbp
    mov rbp, rsp

    mov rax, rsi
    test rdi, rdi
    jz .out

    push rbx
    push r12
    push r13

    mov rbx, rdi        ; list
    mov r12, rsi        ; acc
    mov r13, rdx        ; pred

    mov rdi, [rbx]
    call r13
    test rax, rax
    jz .skip

    mov rdi, [rbx]
    mov rsi, r12
    call add_element
    mov rsi, rax

.skip:
    mov rdi, [rbx + 8]
    mov rdx, r13
    call f

    pop r13
    pop r12
    pop rbx

.out:
    mov rsp, rbp
    pop rbp
    ret

;;; main
_main:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 8

    xor rax, rax
    mov rbx, data_length

.adding_loop:
    lea rdx, [rel data]
    mov rdi, [rdx + rbx*8 - 8]
    mov rsi, rax
    call add_element
    dec rbx
    jnz .adding_loop

    mov rbx, rax

    mov rdi, rax
    lea rsi, [rel print_int]
    call m

    lea rdi, [rel empty_str]
    call _puts

    lea rdx, [rel p]
    xor rsi, rsi
    mov rdi, rbx
    call f

    mov rdi, rax
    lea rsi, [rel print_int]
    call m

    lea rdi, [rel empty_str]
    call _puts

    add rsp, 8
    pop rbx
    pop rbp
    xor rax, rax
    ret

