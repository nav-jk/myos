#include "screen.h"
#include "../kernel/util.h"
#include "../io/port.h"

static int get_screen_offset(int col, int row)
{
    return 2 * (row * MAX_COLS + col);
}

static int get_cursor(void)
{
    int offset;

    port_byte_out(REG_SCREEN_CTRL, 14);
    offset = port_byte_in(REG_SCREEN_DATA) << 8;

    port_byte_out(REG_SCREEN_CTRL, 15);
    offset |= port_byte_in(REG_SCREEN_DATA);

    return offset * 2;
}

static void set_cursor(int offset)
{
    int position = offset / 2;

    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(position >> 8));

    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(position & 0xFF));
}

static void handle_scrolling(void)
{
    int offset;
    int i;

    offset = get_cursor();

    if (offset < MAX_ROWS * MAX_COLS * 2)
        return;

    memory_copy(
        (char *)(VIDEO_ADDRESS + MAX_COLS * 2),
        (char *)VIDEO_ADDRESS,
        (MAX_ROWS - 1) * MAX_COLS * 2
    );

    for (i = 0; i < MAX_COLS; i++)
    {
        int screen_offset = get_screen_offset(i, MAX_ROWS - 1);

        ((unsigned char *)VIDEO_ADDRESS)[screen_offset] = ' ';
        ((unsigned char *)VIDEO_ADDRESS)[screen_offset + 1] =
            WHITE_ON_BLACK;
    }

    set_cursor((MAX_ROWS - 1) * MAX_COLS * 2);
}

void print_char(char character, int col, int row, char attribute_byte)
{
    unsigned char *video_memory;
    int offset;

    if (attribute_byte == 0)
        attribute_byte = WHITE_ON_BLACK;

    video_memory = (unsigned char *)VIDEO_ADDRESS;

    if (col >= 0 && row >= 0)
        offset = get_screen_offset(col, row);
    else
        offset = get_cursor();

    if (character == '\n')
    {
        offset = get_screen_offset(
            0,
            offset / (MAX_COLS * 2) + 1
        );
    }
    else
    {
        video_memory[offset] = character;
        video_memory[offset + 1] = attribute_byte;
        offset += 2;
    }

    set_cursor(offset);
    handle_scrolling();
}

void print_at_color(char *message, int col, int row, char attribute_byte)
{
    int i = 0;

    if (col >= 0 && row >= 0)
        set_cursor(get_screen_offset(col, row));

    while (message[i] != '\0')
    {
        print_char(message[i], -1, -1, attribute_byte);
        i++;
    }
}

void print_at(char *message, int col, int row)
{
    print_at_color(message, col, row, WHITE_ON_BLACK);
}

void print(char *message)
{
    print_at(message, -1, -1);
}

void clear_screen(void)
{
    unsigned char *video_memory;
    int i;

    video_memory = (unsigned char *)VIDEO_ADDRESS;

    for (i = 0; i < MAX_ROWS * MAX_COLS; i++)
    {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = WHITE_ON_BLACK;
    }

    set_cursor(0);
}