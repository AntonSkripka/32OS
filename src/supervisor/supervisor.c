#include "vga.h"
#include "idt.h"
#include "paging.h"
#include "region_array.h"
#include "gdt.h"

extern uint64_t region_pml4s[MAX_REGIONS][512];

extern void paging_flush_load(uint64_t pml4_phys);

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
    vga_print("Stage 2: Launching Kernel at 0x800000...\n");

    void (*kernel_entry)() = (void (*)())0x800000;
    kernel_entry();
}