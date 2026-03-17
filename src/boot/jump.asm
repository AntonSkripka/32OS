[BITS 32]
global jump_to_long_mode
extern pml4
extern gdt_ptr_64 

jump_to_long_mode:
    cli
    lgdt [gdt_ptr_64]

    ; 1. PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov eax, pml4
    mov cr3, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    push dword 0x18
    push dword 0x200000
    retf