#include "vga.h"
#include "io.h"

static uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;
static uint32_t vga_row = 0;
static uint32_t vga_column = 0;
static uint8_t  vga_color = 0x07;

void vga_update_hardware_cursor() {
    uint16_t pos = vga_row * 80 + vga_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_set_cursor(uint32_t x, uint32_t y) {
    vga_column = x;
    vga_row = y;

    vga_update_hardware_cursor();
}

uint32_t vga_get_row()    { return vga_row; }
uint32_t vga_get_column() { return vga_column; }

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = fg | (bg << 4);
}

void vga_clear() {
    for (uint32_t i = 0; i < 80 * 25; i++) {
        VGA_BUFFER[i] = (uint16_t)' ' | (uint16_t)vga_color << 8;
    }
    vga_row = 0;
    vga_column = 0;

    vga_update_hardware_cursor();
}

void vga_print(const char* str) {
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i]);
    }
    vga_update_hardware_cursor();
}

void vga_print_hex(uint64_t n) {
    const char *hex_digits = "0123456789ABCDEF";
    vga_print("0x");
    
    #ifdef __x86_64__
        int bits = 60;
    #else
        int bits = 28;
    #endif

    for (int i = bits; i >= 0; i -= 4) {
        vga_putc(hex_digits[(n >> i) & 0xF]);
    }
}

static void vga_scroll() {
    for (int i = 0; i < 80 * 24; i++) {
        VGA_BUFFER[i] = VGA_BUFFER[i + 80];
    }
    for (int i = 80 * 24; i < 80 * 25; i++) {
        VGA_BUFFER[i] = (uint16_t)' ' | ((uint16_t)vga_color << 8);
    }
    vga_row = 24;
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_column = 0;
        vga_row++;
    } else {
        uint16_t attribute = (uint16_t)vga_color << 8;
        VGA_BUFFER[vga_row * 80 + vga_column] = (uint16_t)c | attribute;
        vga_column++;
    }

    if (vga_column >= 80) {
        vga_column = 0;
        vga_row++;
    }
    
    if (vga_row >= 25) {
        vga_scroll(); 
    }
}

void vga_print_center(const char* str, uint32_t y) {
    uint32_t len = 0;
    while (str[len]) len++;
    
    uint32_t x = (len < 80) ? (80 - len) / 2 : 0;
    vga_set_cursor(x, y);
    vga_print(str);
}

void vga_print_reg_centered(const char* label, uint64_t value, int row, int offset_x) {
    vga_set_cursor(offset_x, row);
    vga_print(label);
    vga_print(": ");
    vga_print_hex(value);
}

void vga_init() {
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
}