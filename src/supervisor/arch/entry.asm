[BITS 64]
section .text

global long_mode_entry
extern supervisor_64_main
extern kernel_stack_top

long_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rax, cr0
    and ax, 0xFFFB      
    or ax, 0x2          
    mov cr0, rax
    mov rax, cr4
    or ax, (3 << 9)    
    mov cr4, rax

    mov rsp, kernel_stack_top
    and rsp, -16         
    
    mov rax, .higher_half
    jmp rax

.higher_half:
    call supervisor_64_main

.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 0x4000 
kernel_stack_top:
    global kernel_stack_top