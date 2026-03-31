#ifndef TIMER_H
#define TIMER_H

#include "types.h"

#define APIC_TIMER_VECTOR 0x40

void timer_init(void);
void timer_calibrate(void);
void apic_timer_interrupt(void);
uint64_t timer_get_milliseconds(void);
void rtc_init(void);
void rtc_update(uint64_t delta_ms);
uint64_t rtc_get_seconds(void);

#endif