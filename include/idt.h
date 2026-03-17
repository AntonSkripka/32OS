#ifndef IDT_H
#define IDT_H

#include "types.h"

struct idt_entry_64 {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  ist;      // Interrupt Stack Table
    uint8_t  flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr_64 {
    uint16_t limit;
    uint64_t base; 
} __attribute__((packed));

struct registers_64 {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    uint64_t int_no;   
    uint64_t err_code; 

    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

extern struct idt_entry_64 idt64[256];
extern struct idt_ptr_64   idtp64;

void universal_handler_64(struct registers_64 *regs);

void init_idt_64(void);
void idt_set_gate_64(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags, uint8_t ist);
extern void idt_load_64(struct idt_ptr_64* ptr);

#endif