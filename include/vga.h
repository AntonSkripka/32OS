#ifndef VGA_H
#define VGA_H

#include "types.h"

enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_LIGHT_BROWN = 14,
    VGA_WHITE = 15,
};

void vga_init();
void vga_clear();
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putc(char c);
void vga_print(const char* str);
void vga_print_hex(uint64_t n);
void vga_print_center(const char* str, uint32_t y);
void vga_print_reg_centered(const char* label, uint64_t value, int row, int offset_x);

void vga_set_cursor(uint32_t x, uint32_t y);
uint32_t vga_get_row();
uint32_t vga_get_column();
void vga_update_hardware_cursor();

#endif