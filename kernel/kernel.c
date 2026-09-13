#include "../drivers/screen.h"
#include "../drivers/keyboard.h"

#define BLACK          0x00
#define BLUE           0x01
#define GREEN          0x02
#define CYAN           0x03
#define RED            0x04
#define MAGENTA        0x05
#define BROWN          0x06
#define LIGHT_GRAY     0x07
#define DARK_GRAY      0x08
#define LIGHT_BLUE     0x09
#define LIGHT_GREEN    0x0A
#define LIGHT_CYAN     0x0B
#define LIGHT_RED      0x0C
#define LIGHT_MAGENTA  0x0D
#define YELLOW         0x0E
#define WHITE          0x0F

void main(void)
{
    char c;

    clear_screen();

    print_at_color(
        "========================================",
        20, 3, LIGHT_CYAN
    );

    print_at_color(
        "          myos KERNEL BOOT",
        20, 4, WHITE
    );

    print_at_color(
        "========================================",
        20, 5, LIGHT_CYAN
    );

    print_at_color(
        "[ OK ]",
        5, 8, LIGHT_GREEN
    );

    print_at_color(
        " BIOS initialization complete",
        12, 8, LIGHT_GRAY
    );

    print_at_color(
        "[ OK ]",
        5, 9, LIGHT_GREEN
    );

    print_at_color(
        " Loading kernel into memory",
        12, 9, LIGHT_GRAY
    );

    print_at_color(
        "[ OK ]",
        5, 10, LIGHT_GREEN
    );

    print_at_color(
        " GDT initialized",
        12, 10, LIGHT_GRAY
    );

    print_at_color(
        "[ OK ]",
        5, 11, LIGHT_GREEN
    );

    print_at_color(
        " Entered protected mode",
        12, 11, LIGHT_GRAY
    );

    print_at_color(
        "[ OK ]",
        5, 12, LIGHT_GREEN
    );

    print_at_color(
        " VGA driver initialized",
        12, 12, LIGHT_GRAY
    );

    print_at_color(
        "[ OK ]",
        5, 13, LIGHT_GREEN
    );

    print_at_color(
        " Screen driver loaded",
        12, 13, LIGHT_GRAY
    );

    print_at_color(
        "[ OK ]",
        5, 14, LIGHT_GREEN
    );

    print_at_color(
        " Keyboard driver initialized",
        12, 14, LIGHT_GRAY
    );

    print_at_color(
        "CPU: x86",
        5, 17, LIGHT_BLUE
    );

    print_at_color(
        "MEM: 640 KB",
        5, 18, YELLOW
    );

    print_at_color(
        "VID: VGA TEXT 80x25",
        5, 19, LIGHT_MAGENTA
    );

    print_at_color(
        "Keyboard ready.",
        5, 21, LIGHT_GREEN
    );

    print_at_color(
        "> ",
        5, 22, WHITE
    );

    keyboard_init();

    while (1)
    {
        c = keyboard_getchar();

        if (c == '\b')
        {
            print_char('\b', -1, -1, WHITE);
        }
        else
        {
            print_char(c, -1, -1, WHITE);
        }
    }
}