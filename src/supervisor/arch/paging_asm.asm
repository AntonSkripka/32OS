[BITS 64]
global paging_flush_load

section .text

paging_flush_load:  
    mov ecx, 0xC0000080         ; EFER MSR
    rdmsr                       
    or eax, (1 << 11)           
    wrmsr

    mov cr3, rdi

    mfence
    jmp .flush
.flush:
    ret