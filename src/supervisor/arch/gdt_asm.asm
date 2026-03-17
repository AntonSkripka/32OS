[BITS 64]
section .text
global gdt_flush_64

gdt_flush_64:
    lgdt [rdi]          
    mov ax, 0x10        ; Kernel Data Selector (0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    push 0x08           ; Kernel Code Selector (0x08)
    lea rax, [rel .next]
    push rax
    retfq               ; Far return (64-bit)
.next:
    mov ax, 0x28        ; TSS Selector
    ltr ax              ; Load Task Register
    ret