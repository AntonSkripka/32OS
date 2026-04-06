#ifndef IO_H
#define IO_H

#include "types.h"
#include "apic.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIT_TIMER_CHANNEL0 0x40
#define PIT_COMMAND        0x43
#define PIT_FREQ           1193182

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

#define APIC_DEFAULT_BASE 0xFEE00000

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

/* I/O APIC Physical & Virtual Bases */
#define IOAPIC_PHYS_BASE  0xFEC00000
#define IOAPIC_VIRT_BASE  (0xFFFFFFFF80000000ULL + 0x1010000)

/* I/O APIC MMIO Register Indices (in 32-bit words) */
#define IOAPIC_IOREGSEL   0
#define IOAPIC_IOWIN      4

#define COM1 0x3F8

static inline void ioapic_write(uint32_t reg, uint32_t data) {
    volatile uint32_t *base = (volatile uint32_t*)IOAPIC_VIRT_BASE;
    base[IOAPIC_IOREGSEL] = (reg & 0xFF);
    base[IOAPIC_IOWIN] = data;
}

static inline uint32_t ioapic_read(uint32_t reg) {
    volatile uint32_t *base = (volatile uint32_t*)IOAPIC_VIRT_BASE;
    base[IOAPIC_IOREGSEL] = (reg & 0xFF);
    return base[IOAPIC_IOWIN];
}

#endif