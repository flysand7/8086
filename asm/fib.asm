bits 16
org 0x7c00

;; Calculating fibonacci sum
;; INPUTS:
;;  cx = number of iterations for F(n)
;; START:
;;  bx = F(0) = 0
;;  ax = F(1) = 1
;; RESULT:
;;  ax = F(n)

mov cx, 10
mov bx, 0
mov dx, 0
mov ax, 1

; Check if we're doing F(0)
cmp cx, 0
jg .test_one
    mov ax, 0
    jmp .end
.test_one:
;; Check if we're doing F(1)
sub cx, 1
jg .loop_fib
    mov ax, 1
    jmp .end
.loop_fib:
    mov dx, bx
    mov bx, ax
    add ax, dx
    sub cx, 1
    jz .end
    jmp .loop_fib
.end: