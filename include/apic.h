#ifndef APIC_H
#define APIC_H

#include "types.h"

#define LAPIC_MSR 0x1B

#define APIC_DEFAULT_BASE 0xFEE00000
#define APIC_VIRT_BASE 0x00A00000

#define LAPIC_REG_ID            0x020
#define LAPIC_REG_VERSION       0x030
#define LAPIC_REG_TPR           0x080
#define LAPIC_REG_EOI           0x0B0
#define LAPIC_REG_SVR           0x0F0
#define LAPIC_REG_DFR           0x0E0
#define LAPIC_REG_LDR           0x0D0
#define LAPIC_REG_LVT_TIMER     0x320
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER_CURRCNT 0x390
#define LAPIC_REG_TIMER_DIV     0x3E0

#define LAPIC_SVR_ENABLE        0x100

#define LAPIC_TIMER_MODE_ONE_SHOT 0x00000000
#define LAPIC_TIMER_MODE_PERIODIC 0x00020000

void apic_init(void);
void apic_disable_legacy_pic(void);
void apic_timer_start_raw(uint32_t initial_count, uint32_t lvt_flags);
void apic_timer_start_ms(uint32_t ms);
void apic_timer_stop(void);
void apic_send_eoi(void);
uint32_t apic_timer_current_count(void);

void apic_set_ticks_per_ms(uint32_t ticks);
uint32_t apic_get_ticks_per_ms(void);

#endif