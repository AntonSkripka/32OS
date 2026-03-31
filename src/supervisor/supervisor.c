#include "vga.h"
#include "idt.h"
#include "paging.h"
#include "region_array.h"
#include "gdt.h"
#include "io.h"
#include "apic.h"
#include "timer.h"

extern uint64_t region_pml4s[MAX_REGIONS][512];
extern volatile uint32_t *lapic_base;

extern void paging_flush_load(uint64_t pml4_phys);

static inline int serial_transmit_empty(void) {
    return (inb(COM1 + 5) & 0x20) != 0;
}

static void serial_putc(char c) {
    while (!serial_transmit_empty());
    outb(COM1, (uint8_t)c);
}

static void serial_print(const char *s) {
    for (uint32_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == '\n') {
            serial_putc('\r');
        }
        serial_putc(s[i]);
    }
}

void supervisor_register_kernel() {
    vga_print("Supervisor: Hardening Memory (4KB Isolation Mode)...\n");

    uint32_t kernel_id = 0;
    uint32_t code_start  = 0x800000; 
    uint32_t state_start = 0xB00000;
    uint32_t state_limit = 0x100000;

    region_init_all(); 
    region_array[kernel_id].access_key = 0xABCDE001;
    paging_init_static();
    paging_flush_load((uint64_t)&region_pml4s[kernel_id]);
    
    vga_print("Supervisor: Paging updated. Region 0 is now isolated.\n");
}

void supervisor_64_main() {
    init_gdt_final();
    init_idt_64(); 
    
    vga_clear();
    vga_print("32OS: 64-bit Supervisor is active.\n");

    supervisor_register_kernel();

    apic_init();
    rtc_init();
    timer_init();

    // Check current count
    extern volatile uint32_t *lapic_base;
    uint32_t curr = lapic_base[LAPIC_REG_TIMER_CURRCNT / 4];
    vga_print("APIC Timer Current Count: ");
    vga_print_hex(curr);
    vga_print("\n");

    serial_print("before sti\n");
    __asm__("sti"); // Enable interrupts after APIC is initialized and PIC disabled
    serial_print("after sti\n");

    vga_print("Stage 2: Launching Kernel at 0x800000...\n");

    void (*kernel_entry)() = (void (*)())0x800000;
    kernel_entry();
}