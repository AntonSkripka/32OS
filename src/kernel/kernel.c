#include "vga.h"

void kernel_main() {
    vga_init();
    vga_print("Hello from kernel");
    while(1);
};