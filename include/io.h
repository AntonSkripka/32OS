#ifndef IO_H
#define IO_H

#include "types.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi) {
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static inline void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi) {
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

#define IOAPIC_BASE 0xFEC00000

static inline void ioapic_write(uint32_t reg, uint32_t data) {
    volatile uint32_t *base = (volatile uint32_t*)IOAPIC_BASE;
    base[0] = (reg & 0xFF);      // IOREGSEL
    base[4] = data;              // IOWIN
}

static inline uint32_t ioapic_read(uint32_t reg) {
    volatile uint32_t *base = (volatile uint32_t*)IOAPIC_BASE;
    base[0] = (reg & 0xFF);
    return base[4];
}

#endif