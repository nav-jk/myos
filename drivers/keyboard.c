#include "keyboard.h"
#include "../io/port.h"
#include "../drivers/screen.h"

#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

#define KEYBOARD_OUTPUT_FULL  0x01

#define SCAN_RELEASE           0x80
#define SCAN_EXTENDED          0xE0

static int shift_pressed = 0;
static int caps_lock = 0;
static int extended = 0;

static char keyboard_buffer[128];
static int buffer_head = 0;
static int buffer_tail = 0;

static const char scan_code_map[128] =
{
    0,
    27,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',
    0,
    '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '/',
    0,
    '*',
    0,
    ' '
};

static const char scan_code_shift_map[128] =
{
    0,
    27,
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}', '\n',
    0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"', '~',
    0,
    '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M',
    '<', '>', '?',
    0,
    '*',
    0,
    ' '
};

static void buffer_put(char c)
{
    int next;

    next = (buffer_head + 1) % 128;

    if (next == buffer_tail)
        return;

    keyboard_buffer[buffer_head] = c;
    buffer_head = next;
}

static char buffer_get(void)
{
    char c;

    if (buffer_head == buffer_tail)
        return 0;

    c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % 128;

    return c;
}

static char translate_scan_code(unsigned char scan_code)
{
    char c;

    if (scan_code >= 128)
        return 0;

    if (shift_pressed)
        c = scan_code_shift_map[scan_code];
    else
        c = scan_code_map[scan_code];

    if (caps_lock && c >= 'a' && c <= 'z')
    {
        if (shift_pressed)
            c = scan_code_map[scan_code];
        else
            c = scan_code_shift_map[scan_code];
    }

    return c;
}

static void process_scan_code(unsigned char scan_code)
{
    char c;

    if (scan_code == SCAN_EXTENDED)
    {
        extended = 1;
        return;
    }

    if (extended)
    {
        extended = 0;
        return;
    }

    if (scan_code == 0x2A || scan_code == 0x36)
    {
        shift_pressed = 1;
        return;
    }

    if (scan_code == 0xAA || scan_code == 0xB6)
    {
        shift_pressed = 0;
        return;
    }

    if (scan_code == 0x3A)
    {
        caps_lock = !caps_lock;
        return;
    }

    if (scan_code & SCAN_RELEASE)
        return;

    c = translate_scan_code(scan_code);

    if (c != 0)
        buffer_put(c);
}

void keyboard_init(void)
{
    shift_pressed = 0;
    caps_lock = 0;
    extended = 0;
    buffer_head = 0;
    buffer_tail = 0;
}

void keyboard_read(void)
{
    unsigned char scan_code;

    while (port_byte_in(KEYBOARD_STATUS_PORT) & KEYBOARD_OUTPUT_FULL)
    {
        scan_code = port_byte_in(KEYBOARD_DATA_PORT);
        process_scan_code(scan_code);
    }
}

char keyboard_getchar(void)
{
    while (buffer_head == buffer_tail)
        keyboard_read();

    return buffer_get();
}