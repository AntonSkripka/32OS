#include "types.h"
#include "vga.h"

extern void init_gdt_prepare_jump();
extern void jump_to_long_mode(); 

uint64_t pml4[512] __attribute__((aligned(4096)));
uint64_t pdpt[512] __attribute__((aligned(4096)));
uint64_t pd[512]   __attribute__((aligned(4096)));

static void setup_minimal_paging() {
    for(int i = 0; i < 512; i++) {
        pml4[i] = 0;
        pdpt[i] = 0;
    }

    pml4[0] = (uintptr_t)pdpt | 0x03; 
    pdpt[0] = (uintptr_t)pd | 0x03;

    for(uint64_t i = 0; i < 512; i++) {
        pd[i] = (i * 0x200000ULL) | 0x83; 
    }
}

void supervisor_32_start() {
    vga_init();
    vga_print("32OS\n");
    vga_print("Load GDT\n");

    init_gdt_prepare_jump(); 
    setup_minimal_paging();

    vga_print("Jump to Long Mode\n");
    jump_to_long_mode(); 
}