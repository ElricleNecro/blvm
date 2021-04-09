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

print_i64:
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

print_f64:
	; extracting from BLISP stack
	mov rsi, [stack_top]
	sub rsi, BLISP_WORD_SIZE
	mov rax, [rsi]
	mov [stack_top], rsi
	; rax has the value we need to print

	; write(STDOUT, buf, buf_size)
	mov rax, SYS_WRITE
	mov rdi, STDOUT
	mov rsi, rsp
	mov rdx, rbx
	syscall
	add rsp, rbx
	ret

; vim:set ft=nasm:
