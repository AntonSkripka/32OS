#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "idt.h"

/* Keyboard Buffer Configuration */
#define KEYBOARD_BUFFER_SIZE 128

typedef struct {
    volatile uint8_t buffer[KEYBOARD_BUFFER_SIZE];  /* Data must be volatile */
    volatile uint16_t head;  /* Write pointer (updated by ISR) */
    volatile uint16_t tail;  /* Read pointer (updated by main loop) */
} keyboard_buffer_t;

/* Keyboard state structure */
typedef struct {
    volatile uint8_t shift_pressed;  /* 1 if Shift is pressed */
    volatile uint8_t caps_lock_on;   /* 1 if Caps Lock is active */
} keyboard_state_t;

/* Circular Buffer API */
int kb_write(uint8_t scancode);      /* Write scancode to buffer, return 0 on success, -1 if full */
int kb_read(void);                   /* Read scancode from buffer, return scancode or -1 if empty */
int kb_is_empty(void);               /* Check if buffer is empty */
int kb_is_full(void);                /* Check if buffer is full */

/* 8042 Controller Initialization */
void kb_8042_init(void);             /* Initialize PS/2 controller and enable IRQ1 */

/* Scancode Conversion */
char scancode_to_ascii(uint8_t scancode);  /* Convert Scancode Set 1 to ASCII */

/* Keyboard Interrupt Handler (called from ISR context) */
void keyboard_interrupt_handler(struct registers_64 *regs);

/* Keyboard state accessors */
extern keyboard_buffer_t kb_buffer;
extern keyboard_state_t kb_state;

/* Syscall numbers for keyboard services (called from kernel region) */
#define SYSCALL_KB_READ              0x10
#define SYSCALL_KB_IS_EMPTY          0x11
#define SYSCALL_KB_INIT              0x12
#define SYSCALL_KB_SCANCODE_TO_ASCII 0x13  /* New: pass scancode, get ascii with shift/caps state */

#endif
