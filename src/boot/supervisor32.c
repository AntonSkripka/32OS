#include "types.h"
#include "vga.h"

extern void init_gdt_prepare_jump();
extern void jump_to_long_mode(); 

uint64_t pml4[512] __attribute__((aligned(4096)));
uint64_t pdpt[512] __attribute__((aligned(4096)));
uint64_t pd[512]   __attribute__((aligned(4096)));

#define KERNEL_LMA 0x100000ULL
#define KERNEL_VMA 0xFFFFFFFF80000000ULL
#define PML4_IDX(addr) (((addr) >> 39) & 0x1FF)

static void setup_minimal_paging() {
    for(int i = 0; i < 512; i++) pml4[i] = pdpt[i] = pd[i] = 0;

    pml4[0] = (uint32_t)pdpt | 0x03;
    pdpt[0] = (uint32_t)pd   | 0x03;

    uint64_t k_pml4_idx = (KERNEL_VMA >> 39) & 0x1FF; // 511
    uint64_t k_pdpt_idx = (KERNEL_VMA >> 30) & 0x1FF; // 510

    pml4[k_pml4_idx] = (uint32_t)pdpt | 0x03;
    pdpt[k_pdpt_idx] = (uint32_t)pd   | 0x03;

    for(uint64_t i = 0; i < 512; i++) {
        pd[i] = (i * 0x200000ULL) | 0x83; // Huge page, Present, Write
    }
}

void supervisor_32_start() {
    init_gdt_prepare_jump(); 
    setup_minimal_paging();
    jump_to_long_mode(); 
}