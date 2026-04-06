#include "idt.h"
#include "vga.h"
#include "timer.h"
#include "keyboard.h"

struct idt_entry_64 idt64[256];
struct idt_ptr_64 idtp64;

const char *exception_messages[] = {
    "Division By Zero",                // 0
    "Debug",                           // 1
    "Non Maskable Interrupt",          // 2
    "Breakpoint",                      // 3
    "Into Detected Overflow",          // 4
    "Out of Bounds",                   // 5
    "Invalid Opcode",                  // 6
    "No Coprocessor",                  // 7
    "Double Fault",                    // 8
    "Coprocessor Segment Overrun",     // 9
    "Bad TSS",                         // 10
    "Segment Not Present",             // 11
    "Stack Fault",                     // 12
    "General Protection Fault",        // 13
    "Page Fault",                      // 14
    "Unknown Interrupt",               // 15
    "Coprocessor Fault",               // 16
    "Alignment Check",                 // 17
    "Machine Check",                   // 18
    "SIMD Floating-Point Exception",   // 19
    "Virtualization Exception",        // 20
    "Control Protection Exception",    // 21
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved"             // 31
};

void handle_supervisor_call(struct registers_64 *regs) {
    uint64_t syscall_num = regs->rax;
    if (syscall_num == 0x01) {
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        vga_print("\n[Supervisor] Kernel requested service 0x01\n");
    } else if (syscall_num == 0x02) {
        regs->rax = 0xABCDEF;
    } else if (syscall_num == 0x03) {
        regs->rax = timer_get_milliseconds();
        regs->rbx = regs->rax;
    } else if (syscall_num == 0x10) {
        regs->rax = kb_read();
    } else if (syscall_num == 0x11) {
        regs->rax = kb_is_empty();
    } else if (syscall_num == 0x12) {
        kb_8042_init();
        regs->rax = 0;
    } else if (syscall_num == 0x13) {
        uint8_t scancode = (uint8_t)(regs->rbx & 0xFF);
        regs->rax = (uint64_t)(unsigned char)scancode_to_ascii(scancode);
    }
}


void idt_set_gate_64(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags, uint8_t ist) {
    idt64[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt64[num].base_mid  = (uint16_t)((base >> 16) & 0xFFFF);
    idt64[num].base_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    idt64[num].sel       = sel;
    idt64[num].ist       = ist & 0x07;
    idt64[num].flags     = flags;
    idt64[num].reserved  = 0;
}

extern void apic_timer_interrupt(void);
extern void keyboard_interrupt_handler(struct registers_64 *regs);

void universal_handler_64(struct registers_64 *regs) {
    if (regs->int_no == APIC_TIMER_VECTOR) {
        apic_timer_interrupt();
        return;
    }

    if (regs->int_no == 0x41) {
        keyboard_interrupt_handler(regs);
        return;
    }

    if (regs->int_no == 0x32) {
        handle_supervisor_call(regs);
        return;
    }

    if (regs->int_no >= 32) {
        vga_print("Unhandled IRQ=");
        vga_print_hex(regs->int_no);
        vga_print("\n");
    }

    __asm__ volatile("cli");

    const uint64_t SUPERVISOR_BOUNDARY = 0xFFFFFFFF80800000;
    vga_clear();

    if (regs->rip < SUPERVISOR_BOUNDARY) {
        vga_set_color(VGA_RED, VGA_BLACK);
        vga_print_center("!!! SUPERVISOR CRITICAL ERROR !!!", 1);
    } else {
        vga_set_color(VGA_WHITE, VGA_RED);
        vga_clear();
        vga_print_center("REGION 0 (KERNEL) CRITICAL ERROR", 1);
    }

    if (regs->int_no < 32) {
        vga_print_center(exception_messages[regs->int_no], 2);
    } else {
        vga_print_center("USER DEFINED INTERRUPT", 2);
    }

    vga_set_cursor(0, 3);
    vga_print("--------------------------------------------------------------------------------");

    int col_left = 23; 
    vga_print_reg_centered("EXCEPTION   ", regs->int_no,   5, col_left);
    vga_print_reg_centered("ERROR CODE  ", regs->err_code, 6, col_left);
    
    if (regs->int_no == 14) {
        uint64_t cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
        vga_print_reg_centered("CR2 (ADDR)  ", cr2, 7, col_left);

        vga_set_cursor(col_left, 8);
        vga_print("CAUSE: ");
        vga_print((regs->err_code & 1) ? "Prot Violation " : "Not Present ");
        vga_print((regs->err_code & 2) ? "Write " : "Read ");
    }
    vga_print_reg_centered("RIP (INST)  ", regs->rip, 10, col_left);

    vga_set_cursor(0, 11);
    vga_print("--------------------------------------------------------------------------------");

    int col1 = 15;
    int col2 = 42;

    vga_print_reg_centered("RAX", regs->rax, 13, col1);
    vga_print_reg_centered("RBX", regs->rbx, 13, col2);
    
    vga_print_reg_centered("RCX", regs->rcx, 14, col1);
    vga_print_reg_centered("RDX", regs->rdx, 14, col2); 

    vga_print_reg_centered("RDI", regs->rdi, 15, col1);
    vga_print_reg_centered("RSI", regs->rsi, 15, col2);

    vga_print_reg_centered("RBP", regs->rbp, 16, col1);
    vga_print_reg_centered("R8 ", regs->r8,  16, col2); 

    vga_set_cursor(0, 24); 

    for(;;);
}

extern uint64_t isr_stub_table[];

void init_idt_64() {
    idtp64.limit = (sizeof(struct idt_entry_64) * 256) - 1;
    idtp64.base  = (uint64_t)&idt64;

    for (uint8_t i = 0; i < 32; i++) {
        uint8_t ist = 0;
        if (i == 8 || i == 14) ist = 1; 
        
        idt_set_gate_64(i, isr_stub_table[i], 0x08, 0x8E, ist);
    }

    for (int i = 32; i < 256; i++) {
        idt_set_gate_64(i, isr_stub_table[i], 0x08, 0x8E, 0);
    }

    idt_set_gate_64(0x32, isr_stub_table[0x32], 0x08, 0xEE, 0);
    idt_set_gate_64(APIC_TIMER_VECTOR, isr_stub_table[APIC_TIMER_VECTOR], 0x08, 0x8E, 0);
    idt_set_gate_64(0x41, isr_stub_table[0x41], 0x08, 0x8E, 0);

    idt_load_64(&idtp64);
}