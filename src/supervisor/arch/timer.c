#include "timer.h"
#include "apic.h"
#include "io.h"
#include "vga.h"
#include "types.h"
#include "serial.h"

static volatile uint64_t system_millis = 0;
static volatile uint32_t rtc_h = 0;
static volatile uint32_t rtc_m = 0;
static volatile uint32_t rtc_s = 0;
static volatile uint32_t rtc_subms = 0;

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

static uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

void rtc_init(void) {
    // wait for update complete clear
    while (cmos_read(0x0A) & 0x80);

    uint8_t sec = cmos_read(0x00);
    uint8_t min = cmos_read(0x02);
    uint8_t hour = cmos_read(0x04);
    uint8_t status = cmos_read(0x0B);

    if (!(status & 0x04)) {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hour = bcd_to_bin(hour);
    }

    rtc_h = hour;
    rtc_m = min;
    rtc_s = sec;

    vga_print("RTC initialized: ");
    vga_print_hex(rtc_h);
    vga_print(":");
    vga_print_hex(rtc_m);
    vga_print(":");
    vga_print_hex(rtc_s);
    vga_print("\n");
}

uint64_t rtc_get_seconds(void) {
    return (uint64_t)rtc_h * 3600 + (uint64_t)rtc_m * 60 + rtc_s;
}

uint64_t timer_get_milliseconds(void) {
    return system_millis;
}

static void pit_wait_count(uint16_t count) {
    outb(PIT_COMMAND, 0x30); // channel0, lobyte/hibyte, mode0
    outb(PIT_TIMER_CHANNEL0, count & 0xFF);
    outb(PIT_TIMER_CHANNEL0, (count >> 8) & 0xFF);

    while (1) {
        outb(PIT_COMMAND, 0x00); // latch channel0
        uint8_t lo = inb(PIT_TIMER_CHANNEL0);
        uint8_t hi = inb(PIT_TIMER_CHANNEL0);
        uint16_t now = (hi << 8) | lo;
        if (now == 0) break;
    }
}

static uint16_t pit_ms_count(uint32_t ms) {
    uint32_t count = (PIT_FREQ * ms + 500) / 1000;
    if (count == 0) count = 1;
    if (count > 0xFFFF) count = 0xFFFF;
    return (uint16_t)count;
}

static void pit_wait_ms(uint32_t ms) {
    pit_wait_count(pit_ms_count(ms));
}

static void print_time(void);
static void update_time_seconds(void);

void timer_calibrate(void) {
    const uint32_t calibration_ms = 10;
    const uint32_t initial_count = 0xFFFFFFFF;

    apic_timer_stop();
    apic_set_ticks_per_ms(0);

    apic_timer_start_raw(initial_count, LAPIC_TIMER_MODE_ONE_SHOT);
    pit_wait_ms(calibration_ms);

    uint32_t current_count = apic_timer_current_count();
    apic_timer_stop();

    serial_print("APIC current count after 10ms: ");
    serial_print_hex(current_count);
    serial_print("\n");

    uint32_t elapsed = initial_count - current_count;
    uint32_t ticks_per_ms = elapsed / calibration_ms;
    if (current_count == initial_count || ticks_per_ms == 0 || ticks_per_ms > 0xFFFFFF) {
        vga_print("APIC timer calibration out of range, using fallback.\n");
        serial_print("APIC calibration bad, using fallback\n");
        ticks_per_ms = 10000;
    }

    apic_set_ticks_per_ms(ticks_per_ms);
    vga_print("APIC timer calibrated: ");
    vga_print_hex(ticks_per_ms);
    vga_print(" ticks/ms\n");
    serial_print("APIC ticks/ms = ");
    serial_print_hex(ticks_per_ms);
    serial_print("\n");
}

void timer_init(void) {
    serial_init();
    serial_print("TIMER init\n");

    timer_calibrate();
    uint32_t ticks_per_ms = apic_get_ticks_per_ms();
    if (ticks_per_ms == 0) {
        ticks_per_ms = 10000;
        apic_set_ticks_per_ms(ticks_per_ms);
    }

    system_millis = 0;
    rtc_subms = 0;

    print_time();
    apic_timer_start_ms(1);
    serial_print("timer started\n");
}

void rtc_update(uint64_t delta_ms) {
    rtc_subms += delta_ms;
    while (rtc_subms >= 1000) {
        rtc_subms -= 1000;
        rtc_s++;
        if (rtc_s >= 60) {
            rtc_s = 0;
            rtc_m++;
            if (rtc_m >= 60) {
                rtc_m = 0;
                rtc_h = (rtc_h + 1) % 24;
            }
        }
    }
}

static void print_time(void) {
    char buffer[9];

    buffer[0] = '0' + (rtc_h / 10);
    buffer[1] = '0' + (rtc_h % 10);
    buffer[2] = ':';
    buffer[3] = '0' + (rtc_m / 10);
    buffer[4] = '0' + (rtc_m % 10);
    buffer[5] = ':';
    buffer[6] = '0' + (rtc_s / 10);
    buffer[7] = '0' + (rtc_s % 10);
    buffer[8] = '\0';

    vga_print_at(0, 22, "Time: ");
    vga_print_at(6, 22, buffer);
}

static void update_time_seconds(void) {
    char seconds[3];
    seconds[0] = '0' + (rtc_s / 10);
    seconds[1] = '0' + (rtc_s % 10);
    seconds[2] = '\0';
    vga_print_at(12, 22, seconds);
}

void apic_timer_interrupt(void) {
    system_millis += 1;
    rtc_update(1);
    update_time_seconds();

    serial_putc('.');

    if ((system_millis % 1000) == 0) {
        print_time();
        serial_putc('T');
    }

    apic_send_eoi();
}
