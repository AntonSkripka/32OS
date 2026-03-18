[bits 64]
global idt_load_64
global isr_common_stub
extern universal_handler_64

%macro ISR_NOERR 1
isr%1:
    push qword 0      
    push qword %1     
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
isr%1:
    push qword %1     
    jmp isr_common_stub
%endmacro

ISR_NOERR 0  ; #DE Divide-by-zero
ISR_NOERR 1  ; #DB Debug
ISR_NOERR 2  ; --- NMI
ISR_NOERR 3  ; #BP Breakpoint
ISR_NOERR 4  ; #OF Overflow
ISR_NOERR 5  ; #BR Bound Range Exceeded
ISR_NOERR 6  ; #UD Invalid Opcode
ISR_NOERR 7  ; #NM Device Not Available
ISR_ERR   8  ; #DF Double Fault (ERR)
ISR_NOERR 9  ; --- Coprocessor Segment Overrun
ISR_ERR   10 ; #TS Invalid TSS (ERR)
ISR_ERR   11 ; #NP Segment Not Present (ERR)
ISR_ERR   12 ; #SS Stack-Segment Fault (ERR)
ISR_ERR   13 ; #GP General Protection Fault (ERR)
ISR_ERR   14 ; #PF Page Fault (ERR)
ISR_NOERR 15 ; --- Reserved
ISR_NOERR 16 ; #MF x87 Floating-Point Exception
ISR_ERR   17 ; #AC Alignment Check (ERR)
ISR_NOERR 18 ; #MC Machine Check
ISR_NOERR 19 ; #XM SIMD Floating-Point Exception
ISR_NOERR 20 ; #VE Virtualization Exception
ISR_ERR   21 ; #CP Control Protection Exception (ERR)

ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30 ; #SX Security Exception (ERR)
ISR_NOERR 31

ISR_NOERR 50

isr_common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp          
    
    mov rbp, rsp          
    and rsp, -16          
    
    call universal_handler_64
    
    mov rsp, rbp          
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16           
    iretq

idt_load_64:
    lidt [rdi]
    ret

section .data
global isr_stub_table
isr_stub_table:
%assign i 0
%rep 32
    dq isr%+i
%assign i i+1
%endrep

%rep 18 
    dq 0 
%endrep
    dq isr50