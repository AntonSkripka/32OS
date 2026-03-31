#include "serial.h"
#include "io.h"

static inline int serial_is_transmit_empty() {
    return (inb(COM1 + 5) & 0x20) != 0;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_putc(char c) {
    while (!serial_is_transmit_empty());
    outb(COM1, (uint8_t)c);
}

void serial_print(const char *s) {
    for (uint32_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == '\n') {
            serial_putc('\r');
        }
        serial_putc(s[i]);
    }
}

void serial_print_hex(uint64_t v) {
    const char *hex = "0123456789ABCDEF";
    serial_print("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        serial_putc(hex[(v >> shift) & 0xF]);
    }
}
