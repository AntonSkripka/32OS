#include "vga.h"
#include "idt.h"
#include "paging.h"
#include "region_array.h"
#include "gdt.h"
#include "io.h"
#include "apic.h"
#include "timer.h"
#include "serial.h"

extern uint64_t region_pml4s[MAX_REGIONS][512];
extern volatile uint32_t *lapic_base;

extern void paging_flush_load(uint64_t pml4_phys);

void supervisor_register_kernel() {
    vga_print("Supervisor: Hardening Memory (4KB Isolation Mode)...\n");

    uint32_t kernel_id = 0;

    region_init_all(); 
    vga_print("region init static+\n");
    paging_init_static();
    vga_print("paging init static+\n");
    uint64_t pml4_phys = (uintptr_t)&region_pml4s[kernel_id] - 0xFFFFFFFF80000000ULL;
    paging_flush_load(pml4_phys);
    
    vga_print("Supervisor: Paging updated. Region 0 is now isolated.\n");
}

void supervisor_64_main() {
    init_gdt_final();
    init_idt_64(); 
    
    vga_clear();
    vga_print("32OS: 64-bit Supervisor is active.\n");

    supervisor_register_kernel();

    apic_init();
    
    ioapic_init();
    ioapic_redirect_irq(1, 0x41);
    
    rtc_init();
    timer_init();

    extern volatile uint32_t *lapic_base;
    uint32_t curr = lapic_base[LAPIC_REG_TIMER_CURRCNT / 4];
    vga_print("APIC Timer Current Count: ");
    vga_print_hex(curr);
    vga_print("\n");

    serial_print("before sti\n");
    __asm__("sti");
    serial_print("after sti\n");

    vga_print("Stage 2: Launching Kernel at 0xFFFFFFFF80800000...\n");

    void (*kernel_entry)() = (void (*)())0xFFFFFFFF80800000;
    kernel_entry();
}