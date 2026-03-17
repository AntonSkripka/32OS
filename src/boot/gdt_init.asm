[BITS 32]

section .data
align 16
global boot_gdt
boot_gdt:
    dq 0x0000000000000000          ; 0x00: Null
    dq 0x00cf9a000000ffff          ; 0x08: Code 32
    dq 0x00cf92000000ffff          ; 0x10: Data 32
    dq 0x00af9a000000ffff          ; 0x18: Code 64 
    dq 0x00cf92000000ffff          ; 0x20: Data 64

align 16
global gdt_ptr_64                  
gdt_ptr_64:
    dw 39                          ; Limit 
    dd 0                           ; Base

section .text
global init_gdt_prepare_jump

init_gdt_prepare_jump:
    mov eax, boot_gdt
    mov [gdt_ptr_64 + 2], eax

    lgdt [gdt_ptr_64]

    ; 0x08 - Code 32
    push 0x08
    push .next
    retf

.next:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    
    ret