#include "gdt.h"

struct gdt_entry gdt64[9];
struct gdt_ptr_64 gdt64_ptr;
struct tss_entry tss_inst __attribute__((aligned(16)));

uint8_t ist_stack_panic[16384] __attribute__((aligned(16))); 

void gdt_set_gate_64(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt64[num].limit_low    = (limit & 0xFFFF);
    gdt64[num].base_low     = (base & 0xFFFF);
    gdt64[num].base_middle  = (base >> 16) & 0xFF;
    gdt64[num].access       = access;
    gdt64[num].granularity  = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt64[num].base_high    = (base >> 24) & 0xFF;
}

void gdt_set_tss_64(int num, uint64_t base, uint32_t limit) {
    gdt64[num].limit_low    = (uint16_t)(limit & 0xFFFF);
    gdt64[num].base_low     = (uint16_t)(base & 0xFFFF);
    gdt64[num].base_middle  = (uint8_t)((base >> 16) & 0xFF);
    gdt64[num].access       = 0x89; // Present, Available 64-bit TSS
    gdt64[num].granularity  = (uint8_t)((limit >> 16) & 0x0F);
    gdt64[num].base_high    = (uint8_t)((base >> 24) & 0xFF);

    struct gdt_entry* second_half = &gdt64[num + 1];
    uint32_t* raw_data = (uint32_t*)second_half;
    
    raw_data[0] = (uint32_t)(base >> 32); 
    raw_data[1] = 0;                      
}

void init_gdt_final() {
    // 0: Null
    gdt_set_gate_64(0, 0, 0, 0, 0);

    // 1: Kernel Code (Selector 0x08)
    gdt_set_gate_64(1, 0, 0xFFFFF, 0x9A, 0xAF); 
    // 2: Kernel Data (Selector 0x10)
    gdt_set_gate_64(2, 0, 0xFFFFF, 0x92, 0xAF); 

    // 3: User Data (Selector 0x18)
    gdt_set_gate_64(3, 0, 0xFFFFF, 0xF2, 0xAF);
    // 4: User Code (Selector 0x20)
    gdt_set_gate_64(4, 0, 0xFFFFF, 0xFA, 0xAF);

    for(int i = 0; i < sizeof(tss_inst); i++) ((char*)&tss_inst)[i] = 0;

    uint64_t panic_stack_top = (uint64_t)&ist_stack_panic[16384];

    tss_inst.ist[0] = panic_stack_top & ~0xF; 

    static uint8_t kernel_stack[8192] __attribute__((aligned(16)));

    tss_inst.rsp0 = ((uint64_t)&kernel_stack[8192]) & ~0xF;

    tss_inst.iomap_base = sizeof(struct tss_entry);

    gdt_set_tss_64(5, (uint64_t)&tss_inst, sizeof(struct tss_entry) - 1);

    gdt64_ptr.limit = (sizeof(struct gdt_entry) * 7) - 1; 

    gdt64_ptr.base = (uint64_t)&gdt64;

    gdt_flush_64((uint64_t)&gdt64_ptr);
}