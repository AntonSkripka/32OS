#include "vga.h"

uint64_t call_32os(uint64_t function) {
    uint64_t ret;
    __asm__ volatile (
        "mov %1, %%rax\n\t"
        "int $0x32\n\t"
        "mov %%rax, %0"
        : "=r"(ret)
        : "r"(function)
        : "rax"
    );
    return ret;
}

void kernel_main() {
    vga_init();
    vga_print("Hello from kernel\n");
    call_32os(0x01);
    uint64_t val = call_32os(0x02);
    vga_print_hex(val);
    __asm__("hlt");
};