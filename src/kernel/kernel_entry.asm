[bits 64]
global _start
extern kernel_main

section .text
_start:
    mov rsp, 0xFFFFFFFF80900000
    and rsp, -16
    call kernel_main

.loop:
    hlt
    jmp .loop