#include "apic.h"
#include "io.h"
#include "timer.h"
#include "vga.h"
#include "types.h"
#include "serial.h"

volatile uint32_t *lapic_base = (volatile uint32_t *)APIC_VIRT_BASE;

static uint32_t apic_timer_ticks_per_ms = 0;

static inline void lapic_write(uint32_t reg, uint32_t value) {
    lapic_base[reg / 4] = value;
    (void)lapic_base[reg / 4];
}

static inline uint32_t lapic_read(uint32_t reg) {
    return lapic_base[reg / 4];
}

static bool apic_supported(void) {
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1));
    return (edx & (1 << 9)) != 0;
}

static void apic_map_base(void) {
    uint32_t lo, hi;
    cpu_get_msr(LAPIC_MSR, &lo, &hi);
    uintptr_t base = ((uintptr_t)hi << 32) | (lo & 0xFFFFF000);
    if (base == 0) {
        base = APIC_DEFAULT_BASE;
    }

    // Ensure APIC is enabled
    lo |= (1 << 11);
    cpu_set_msr(LAPIC_MSR, lo, hi);

    // Local APIC is mapped at APIC_VIRT_BASE in supervisor paging.
    // Avoid dereferencing physical address directly which is not identity-mapped.
    if (base != APIC_DEFAULT_BASE) {
        // In this MVP we assume default base; if non-default, MSR value is noted.
        // You can extend paging logic to map this physical base in a future improvement.
    }

    lapic_base = (volatile uint32_t *)APIC_VIRT_BASE;
}

void apic_disable_legacy_pic(void) {
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    vga_print("Legacy PIC disabled.\n");
}

void apic_init(void) {
    if (!apic_supported()) {
        vga_print("Local APIC not supported on this CPU.\n");
        return;
    }

    apic_map_base();

    uint32_t id = lapic_read(LAPIC_REG_ID);
    vga_print("Local APIC detected, ID=");
    vga_print_hex(id >> 24);
    vga_print("\n");

    lapic_write(LAPIC_REG_SVR, LAPIC_SVR_ENABLE | 0xFF);

    lapic_write(LAPIC_REG_DFR, 0xFFFFFFFF); // flat model
    lapic_write(LAPIC_REG_LDR, 0xFFFFFFFF); // logical destination
    lapic_write(LAPIC_REG_TPR, 0);         // accept all interrupt priorities

    apic_disable_legacy_pic();
}

void apic_send_eoi(void) {
    lapic_write(LAPIC_REG_EOI, 0);
}

void apic_timer_start_raw(uint32_t initial_count, uint32_t lvt_flags) {
    lapic_write(LAPIC_REG_TIMER_DIV, 0x3); // divide by 16
    lapic_write(LAPIC_REG_LVT_TIMER, APIC_TIMER_VECTOR | lvt_flags);
    lapic_write(LAPIC_REG_TIMER_INITCNT, initial_count);
}

uint32_t apic_timer_current_count(void) {
    return lapic_read(LAPIC_REG_TIMER_CURRCNT);
}

void apic_timer_start_ms(uint32_t ms) {
    if (apic_timer_ticks_per_ms == 0 || ms == 0) return;

    uint64_t raw_count = (uint64_t)apic_timer_ticks_per_ms * ms;
    if (raw_count == 0) raw_count = 1;
    if (raw_count > 0xFFFFFFFF) raw_count = 0xFFFFFFFF;

    apic_timer_start_raw((uint32_t)raw_count, LAPIC_TIMER_MODE_PERIODIC);

    uint32_t lvt = lapic_read(LAPIC_REG_LVT_TIMER);
    vga_print("APIC LVT_TIMER=");
    vga_print_hex(lvt);
    vga_print("\n");
    serial_print("APIC LVT_TIMER=");
    serial_print_hex(lvt);
}

void apic_timer_stop(void) {
    lapic_write(LAPIC_REG_LVT_TIMER, 1 << 16); // mask
}

void apic_set_ticks_per_ms(uint32_t ticks) {
    apic_timer_ticks_per_ms = ticks;
}

uint32_t apic_get_ticks_per_ms(void) {
    return apic_timer_ticks_per_ms;
}
