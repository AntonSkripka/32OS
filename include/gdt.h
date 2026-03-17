#ifndef GDT_H
#define GDT_H

#include "types.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr_32 {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_ptr_64 {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];    
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

void gdt_set_gate_32(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

void init_gdt_final(void);
void gdt_set_gate_64(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_set_tss_64(int num, uint64_t base, uint32_t limit);

extern void jump_to_long_mode(void);

extern void gdt_flush_64(uint64_t gdt_ptr_addr);

extern void tss_flush_64(void);

#endif