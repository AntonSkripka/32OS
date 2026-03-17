MB_ALIGN     equ  1 << 0
MB_MEMINFO   equ  1 << 1
MB_FLAGS     equ  MB_ALIGN | MB_MEMINFO
MB_MAGIC     equ  0x1BADB002
MB_CHECKSUM  equ  -(MB_MAGIC + MB_FLAGS)

section .multiboot
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 
stack_top:

section .text
global start
extern supervisor_32_start

start:
    mov esp, stack_top
    
    push ebx 
    push eax 

    call supervisor_32_start
.halt:
    hlt
    jmp .halt