#include "vga.h"
#include "keyboard.h"
#include "serial.h"
#include "types.h"

/* Syscall interface to supervisor's keyboard services */
#define SYSCALL_KB_READ              0x10
#define SYSCALL_KB_IS_EMPTY          0x11
#define SYSCALL_KB_INIT              0x12
#define SYSCALL_KB_SCANCODE_TO_ASCII 0x13

uint64_t call_32os(uint64_t function)
{
    uint64_t ret;
    __asm__ volatile(
        "mov %1, %%rax\n\t"
        "int $0x32\n\t"
        "mov %%rax, %0"
        : "=r"(ret)
        : "r"(function)
        : "rax");
    return ret;
}

static int kb_read_syscall(void) {
    return (int)call_32os(SYSCALL_KB_READ);
}

static int kb_is_empty_syscall(void) {
    return (int)call_32os(SYSCALL_KB_IS_EMPTY);
}

static void kb_8042_init_syscall(void) {
    call_32os(SYSCALL_KB_INIT);
}

static char kb_scancode_to_ascii(uint8_t scancode) {
    uint64_t ret;
    __asm__ volatile(
        "mov %1, %%rbx\n\t"        /* Set RBX = scancode */
        "mov %2, %%rax\n\t"        /* Set RAX = SYSCALL_KB_SCANCODE_TO_ASCII */
        "int $0x32\n\t"            /* Call supervisor */
        "mov %%rax, %0"            /* Return value in RAX */
        : "=r"(ret)
        : "r"((uint64_t)scancode), "i"(SYSCALL_KB_SCANCODE_TO_ASCII)
        : "rax", "rbx");
    return (char)ret;
}

void kernel_main()
{
    vga_init();
    vga_print("Hello from kernel\n");
    kb_8042_init_syscall();
    vga_print("Keyboard ready (via supervisor). Press any keys...\n");
    serial_print("Kernel main: Entering main event loop\n");
    uint32_t kb_write_start_col = vga_get_column();
    uint32_t kb_write_start_row = vga_get_row();

    while (1)
    {
        __asm__ volatile("sti; hlt");

        while (!kb_is_empty_syscall())
        {
            int scancode = kb_read_syscall();
            if (scancode != -1 && scancode < 256)
            {
                char ch = kb_scancode_to_ascii((uint8_t)scancode);
                
                if (ch != 0)
                {
                    if (ch == '\b') {
                        uint32_t col = vga_get_column();
                        uint32_t row = vga_get_row();

                        int is_at_start = (row == kb_write_start_row && col == kb_write_start_col);
                        
                        if (!is_at_start) {
                            if (col > 0) {
                                col--;
                            } else if (row > kb_write_start_row) {
                                row--;
                                col = 79;
                            }

                            vga_set_cursor(col, row);

                            vga_putc_at(col, row, ' ');
                            vga_update_hardware_cursor();
                            
                            serial_putc('\b');
                        }
                    } else {
                        vga_putc(ch);
                        vga_update_hardware_cursor();
                        serial_putc(ch);
                    }
                }
            }
        }
    }
}