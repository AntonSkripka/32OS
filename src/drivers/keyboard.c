#include "keyboard.h"
#include "io.h"
#include "apic.h"
#include "idt.h"
#include "serial.h"
#include "vga.h"

/* ===== 8042 PS/2 Controller I/O Ports ===== */
#define KB_DATA_PORT   0x60  /* Read: scancode, Write: data to 8042 */
#define KB_STATUS_PORT 0x64  /* Read: status, Write: command */

/* 8042 Status Register Bits */
#define KB_STATUS_OUTBUF_FULL  0x01  /* Output buffer has data */
#define KB_STATUS_INBUF_FULL   0x02  /* Input buffer is full */

/* 8042 Commands */
#define KB_CMD_DISABLE_AUX    0xA7  /* Disable PS/2 mouse */
#define KB_CMD_ENABLE_AUX     0xA8  /* Enable PS/2 mouse */
#define KB_CMD_DISABLE_KB     0xAD  /* Disable keyboard */
#define KB_CMD_ENABLE_KB      0xAE  /* Enable keyboard */
#define KB_CMD_READ_CONFIG    0x20  /* Read control byte */
#define KB_CMD_WRITE_CONFIG   0x60  /* Write control byte */

/* ===== Circular Keyboard Buffer ===== */
keyboard_buffer_t kb_buffer = {
    .buffer = {0},
    .head = 0,
    .tail = 0
};

/* ===== Keyboard State ===== */
keyboard_state_t kb_state = {
    .shift_pressed = 0,
    .caps_lock_on = 0
};

/* ===== Scancode-to-ASCII Lookup Table (Scancode Set 1) ===== */
/* Maps scancodes to ASCII characters. Non-printable keys map to 0. */
/* Row 0: Normal (no Shift) */
static const char scancode_to_ascii_table[128] = {
    0,     0x1B,  '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',     /* 0x00-0x09 */
    '9',   '0',   '-',   '=',   '\b',  '\t',  'q',   'w',   'e',   'r',     /* 0x0A-0x13 */
    't',   'y',   'u',   'i',   'o',   'p',   '[',   ']',   '\n',  0,       /* 0x14-0x1D */
    'a',   's',   'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',     /* 0x1E-0x27 */
    '\'',  '`',   0,     '\\',  'z',   'x',   'c',   'v',   'b',   'n',     /* 0x28-0x31 */
    'm',   ',',   '.',   '/',   0,     '*',   0,     ' ',   0,     0,       /* 0x32-0x3B */
    /* Remaining entries: function keys, extended scancodes, etc. */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x3C-0x4B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x4C-0x5B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x5C-0x6B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   /* 0x6C-0x7F */
};

/* Row 1: With Shift */
static const char scancode_to_ascii_shift_table[128] = {
    0,     0x1B,  '!',   '@',   '#',   '$',   '%',   '^',   '&',   '*',     /* 0x00-0x09 */
    '(',   ')',   '_',   '+',   '\b',  '\t',  'Q',   'W',   'E',   'R',     /* 0x0A-0x13 */
    'T',   'Y',   'U',   'I',   'O',   'P',   '{',   '}',   '\n',  0,       /* 0x14-0x1D */
    'A',   'S',   'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',     /* 0x1E-0x27 */
    '\"',  '~',   0,     '|',   'Z',   'X',   'C',   'V',   'B',   'N',     /* 0x28-0x31 */
    'M',   '<',   '>',   '?',   0,     '*',   0,     ' ',   0,     0,       /* 0x32-0x3B */
    /* Remaining entries: function keys, extended scancodes, etc. */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x3C-0x4B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x4C-0x5B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x5C-0x6B */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   /* 0x6C-0x7F */
};

/* Special scancode definitions */
#define KB_LSHIFT_PRESS  0x2A
#define KB_LSHIFT_RELEASE 0xAA
#define KB_RSHIFT_PRESS  0x36
#define KB_RSHIFT_RELEASE 0xB6
#define KB_CAPS_LOCK_PRESS 0x3A
#define KB_CAPS_LOCK_RELEASE 0xBA
#define KB_BACKSPACE     0x0E

/* ===== Circular Buffer Implementation ===== */

int kb_write(uint8_t scancode) {
    uint16_t next_head = (kb_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
    
    /* Buffer full check */
    if (next_head == kb_buffer.tail) {
        serial_print("KB: Buffer overflow!\n");
        return -1;
    }
    
    kb_buffer.buffer[kb_buffer.head] = scancode;
    kb_buffer.head = next_head;
    return 0;
}

int kb_read(void) {
    if (kb_buffer.head == kb_buffer.tail) {
        return -1;  /* Buffer empty */
    }
    
    uint8_t scancode = kb_buffer.buffer[kb_buffer.tail];
    kb_buffer.tail = (kb_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
    return (int)scancode;
}

int kb_is_empty(void) {
    return kb_buffer.head == kb_buffer.tail;
}

int kb_is_full(void) {
    return ((kb_buffer.head + 1) % KEYBOARD_BUFFER_SIZE) == kb_buffer.tail;
}

/* ===== 8042 Controller Initialization ===== */

static void kb_wait_status(uint8_t mask, uint8_t value, int timeout) {
    while (timeout-- > 0) {
        uint8_t status = inb(KB_STATUS_PORT);
        if ((status & mask) == value) {
            return;
        }
        /* Busy wait (could add micro-delay here if needed) */
    }
    serial_print("KB: Timeout waiting for status!\n");
}

void kb_8042_init(void) {
    serial_print("KB: Initializing 8042 controller...\n");
    
    /* Disable keyboard and mouse */
    outb(KB_STATUS_PORT, KB_CMD_DISABLE_KB);
    kb_wait_status(KB_STATUS_INBUF_FULL, 0, 10000);
    
    /* Flush input buffer (drain all pending data) */
    while (inb(KB_STATUS_PORT) & KB_STATUS_OUTBUF_FULL) {
        inb(KB_DATA_PORT);
    }
    
    /* Disable mouse port */
    outb(KB_STATUS_PORT, KB_CMD_DISABLE_AUX);
    kb_wait_status(KB_STATUS_INBUF_FULL, 0, 10000);
    
    /* Read and modify control byte to enable keyboard interrupts (IRQ1) */
    outb(KB_STATUS_PORT, KB_CMD_READ_CONFIG);
    kb_wait_status(KB_STATUS_INBUF_FULL, 0, 10000);
    kb_wait_status(KB_STATUS_OUTBUF_FULL, KB_STATUS_OUTBUF_FULL, 10000);
    uint8_t config = inb(KB_DATA_PORT);
    
    /* Set bit 0 (keyboard interrupt enable) */
    config |= 0x01;
    
    /* Write back modified control byte */
    outb(KB_STATUS_PORT, KB_CMD_WRITE_CONFIG);
    kb_wait_status(KB_STATUS_INBUF_FULL, 0, 10000);
    outb(KB_DATA_PORT, config);
    kb_wait_status(KB_STATUS_INBUF_FULL, 0, 10000);
    
    /* Enable keyboard */
    outb(KB_STATUS_PORT, KB_CMD_ENABLE_KB);
    kb_wait_status(KB_STATUS_INBUF_FULL, 0, 10000);
    
    /* Flush input buffer again */
    while (inb(KB_STATUS_PORT) & KB_STATUS_OUTBUF_FULL) {
        inb(KB_DATA_PORT);
    }
    
    serial_print("KB: 8042 controller initialized successfully.\n");
}

/* ===== Scancode to ASCII Conversion ===== */

static int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

char scancode_to_ascii(uint8_t scancode) {
    /* Filter out key releases (bit 7 set) - these are NOT processed here */
    if (scancode & 0x80) {
        return 0;  /* Key release, ignore */
    }
    
    /* Handle only valid scancode range */
    if (scancode >= 128) {
        return 0;
    }
    
    /* Get base character (without shift/caps consideration) */
    char base_char = scancode_to_ascii_table[scancode];
    
    /* Get shift variant */
    char shift_char = scancode_to_ascii_shift_table[scancode];
    
    /* Backspace is special - always return as-is */
    if (scancode == KB_BACKSPACE) {
        return '\b';
    }
    
    /* Non-letter characters: use shift table directly if shift pressed */
    if (!is_letter(base_char)) {
        if (kb_state.shift_pressed) {
            return shift_char;
        }
        return base_char;
    }

    int use_upper = kb_state.caps_lock_on;  /* Caps Lock makes uppercase */
    if (kb_state.shift_pressed) {
        use_upper = !use_upper;  /* Shift toggles the case */
    }
    
    return use_upper ? shift_char : base_char;
}

/* ===== Keyboard Interrupt Handler ===== */

void keyboard_interrupt_handler(struct registers_64 *regs) {
    /* Read scancode from 8042 data port */
    uint8_t scancode = inb(KB_DATA_PORT);
    
    /* Handle special keys that modify state (don't put in buffer) */
    
    /* Shift key press / release */
    if (scancode == KB_LSHIFT_PRESS || scancode == KB_RSHIFT_PRESS) {
        kb_state.shift_pressed = 1;
        apic_send_eoi();
        return;
    }
    if (scancode == KB_LSHIFT_RELEASE || scancode == KB_RSHIFT_RELEASE) {
        kb_state.shift_pressed = 0;
        apic_send_eoi();
        return;
    }
    
    /* Caps Lock key press - toggle state on each press */
    if (scancode == KB_CAPS_LOCK_PRESS) {
        kb_state.caps_lock_on = !kb_state.caps_lock_on;
        
        /* Debug: log Caps Lock toggle */
        serial_print("CAPS:");
        serial_print(kb_state.caps_lock_on ? "ON" : "OFF");
        serial_putc('\n');
        
        apic_send_eoi();
        return;
    }
    if (scancode == KB_CAPS_LOCK_RELEASE) {
        /* Ignore release event */
        apic_send_eoi();
        return;
    }
    
    /* All other keys go into the buffer for processing */
    uint16_t next_head = (kb_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
    
    if (next_head != kb_buffer.tail) {
        /* Buffer NOT full - write scancode */
        kb_buffer.buffer[kb_buffer.head] = scancode;
        
        /* Memory barrier: ensure scancode is written before head is updated */
        __asm__ volatile("mfence\n\t" : : : "memory");
        
        kb_buffer.head = next_head;
    } else {
        /* Buffer overflow: log and continue */
        serial_print("KB: Buffer overflow!\n");
    }
    
    /* Send End-of-Interrupt to Local APIC */
    apic_send_eoi();
}
