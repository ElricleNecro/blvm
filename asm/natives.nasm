print_u64:
	; extracting from BLISP stack
	mov rsi, [stack_top]
	sub rsi, BLISP_WORD_SIZE
	mov rax, [rsi]
	mov [stack_top], rsi
	; rax has the value we need to print
	mov rdi, 0	; rdi is the counter of chars
	; add new line
	dec rsp
	inc rdi
	mov BYTE [rsp], 10
.loop:
	xor rdx, rdx
	mov rbx, 10
	div rbx
	add rdx, '0'
	dec rsp
	inc rdi
	mov [rsp], dl
	cmp rax, 0
	jne .loop
	; rsp is the beginning of the buffer
	; rdi contains the size of the buf
	mov rbx, rdi
	; write(STDOUT, buf, buf_size)
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, rsp
	mov rdx, rbx
	syscall
	add rsp, rbx
	ret

print_int:
	cmp rax, 0
	jge .skip_negative

	push rax
	mov BYTE [x], '-'
	; write(STDOUT, buf, buf_size)
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, rsp
	mov rdx, rbx
	syscall

	pop rax
	neg rax

.skip_negative:
	; rax has the value we need to print
	mov rdi, 0	; rdi is the counter of chars
	; add new line
	dec rsp
	inc rdi
	mov BYTE [rsp], 10
.loop:
	xor rdx, rdx
	mov rbx, 10
	div rbx
	add rdx, '0'
	dec rsp
	inc rdi
	mov [rsp], dl
	cmp rax, 0
	jne .loop
	; rsp is the beginning of the buffer
	; rdi contains the size of the buf
	mov rbx, rdi
	; write(STDOUT, buf, buf_size)
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, rsp
	mov rdx, rbx
	syscall
	add rsp, rbx
	ret
print_i64:
	; extracting from BLISP stack
	mov rsi, [stack_top]
	sub rsi, BLISP_WORD_SIZE
	mov rax, [rsi]
	mov [stack_top], rsi

	call print_int

;; xmm0: input
;; xmm0: output
fabs:
	pxor xmm1, xmm1
	comisd xmm0, xmm1
	ja .skip
	movsd xmm1, QWORD [nzero]
	pxor xmm0, xmm1
.skip:
	ret

;; xmm0: input
;; xmm0: output
;; uses xmm1 and rax for work
floor:
	cvttsd2si rax, xmm0
	cvtsi2sd xmm1, rax
	subsd xmm0, xmm1	;; testing equality between xmm0 and xmm1 (0)

	pxor xmm1, xmm1
	comisd xmm0, xmm1
	jae .skipdec
	dec rax
.skipdec:
	cvtsi2sd xmm0, rax
	ret

;; xmm0: input
;; xmm0: output
;; frac(x) = x - trunc(x)
frac:
	movsd xmm1, xmm0
	cvttsd2si rax, xmm1
	cvtsi2sd xmm1, rax
	subsd xmm0, xmm1
	ret

;; xmm0 fraction to print, must be < 0.0
print_frac:
	mov BYTE [x], '.'
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, x
	mov rdx, 1
	syscall

%define PRINT_FRAC_N 4
%define M 0
%define R (M + 8)
%define U (R + 8)
%define SIZE (U + 8)

	sub rsp, SIZE

	; Initialise R
	movsd QWORD [rsp + R], xmm0

	; Initialise U
	movsd xmm1, QWORD [ten]
	mulsd xmm0, xmm1
	call floor
	movsd QWORD [rsp + U], xmm0

	; Initialise M to $b^{N - 1}$
	movsd xmm0, QWORD [one]
	mov rax, PRINT_FRAC_N

.loop:
	test rax, rax
	jz .end

	movsd xmm1, QWORD [tenth]
	mulsd xmm0, xmm3
	dec rax
	jmp .loop

.end:
	movsd xmm1, QWORD [half]
	mulsd xmm0, xmm1
	movsd [rsp + M], xmm0

.loop2:
	; U = floor(R * 10.0)
	movsd xmm0, QWORD [rsp + R]
	movsd xmm1, QWORD [ten]
	mulsd xmm0, xmm1
	call floor
	movsd QWORD [rsp + U], xmm0

	; R = frac(R * 10.0)
	movsd xmm0, QWORD [rsp + R]
	movsd xmm1, QWORD [ten]
	mulsd xmm0, xmm1
	call frac
	movsd QWORD [rsp + R], xmm0

	; M *= B
	movsd xmm0, QWORD [rsp + M]
	movsd xmm1, QWORD [ten]
	mulsd xmm0, xmm1
	movsd QWORD [rsp + M], xmm0

	; R < M || R > 1 - M
	; 1. R < M
	movsd xmm0, QWORD [rsp + R]
	movsd xmm1, QWORD [rsp + M]
	comisd xmm0, xmm1
	jb .end2
	; 2. R > 1 - M
	movsd xmm0, QWORD [one]
	movsd xmm1, QWORD [rsp + M]
	subsd xmm0, xmm1
	movsd xmm1, xmm0
	movsd xmm0, QWORD [rsp + R]
	ja .end2

	; if R > 0.5 -> U += 1.0
	movsd xmm0, QWORD [rsp + R]
	movsd xmm1, QWORD [half]
	comisd xmm0, xmm1
	jbe .skipinc

	movsd xmm0, QWORD [rsp + U]
	movsd xmm1, QWORD [one]
	addsd xmm0, xmm1
	movsd QWORD[rsp + U], xmm0

.skipinc:

	; printf("%d", (int)U);
	movsd xmm0, QWORD [rsp + U]
	cvtsd2si rax, xmm0
	add al, '0'
	mov BYTE [x], al
	; -> syscall write(STDOUT, buf, buf_size)
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, x
	mov rdx, 1
	syscall

	jmp .loop2

.end2:

	; printing loop:

	add rsp, SIZE

%undef PRINT_FRAC_N
%undef M
%undef R
%undef U
%undef SIZE

	ret

print_f64:
	; extracting from BLISP stack
	mov rsi, [stack_top]
	sub rsi, BLISP_WORD_SIZE
	movsd xmm0, QWORD [rsi]
	mov [stack_top], rsi
	; rax has the value we need to print

	cvttsd2si rax, xmm0
	call print_int
	call frac
	call fabs
	call print_frac

	mov BYTE [x], 10
	; write(STDOUT, buf, buf_size)
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, x
	mov rdx, 1
	syscall
	ret

; vim:set ft=nasm:
