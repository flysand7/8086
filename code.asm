bits 16
cpu 8086
org 0x7c00

mov ax, 3
mov dx, 0
mov bx, 10

neg ax
not ax