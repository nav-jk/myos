#include "screen.h"
#include "../kernel/util.h"


int get_screen_offset(int col, int row)
{
    return 2 * (row * MAX_COLS + col);
}


// Read the current cursor position from the VGA controller. VGA cursor position is measured in character cells, not bytes.
int get_cursor(void)
{
    int offset;

    /* Select cursor location high byte */
    port_byte_out(REG_SCREEN_CTRL, 14);

    offset = port_byte_in(REG_SCREEN_DATA) << 8;

    /* Select cursor location low byte */
    port_byte_out(REG_SCREEN_CTRL, 15);

    offset += port_byte_in(REG_SCREEN_DATA);

    return offset * 2;
}


void set_cursor(int offset)
{
    /*
     * Convert byte offset into character-cell offset.
     *
     * Example:
     *
     *     160 bytes -> cell 80
     */
    offset /= 2;

    /* Send high byte */
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));

    /* Send low byte */
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xFF));
}


int handle_scrolling(int cursor_offset)
{

    int screen_size = MAX_ROWS * MAX_COLS * 2;

    if (cursor_offset < screen_size)
        return cursor_offset;

    int i;

    for (i = 1; i < MAX_ROWS; i++)
    {
        memory_copy(
            (char *)(VIDEO_ADDRESS + get_screen_offset(0, i)),
            (char *)(VIDEO_ADDRESS + get_screen_offset(0, i - 1)),
            MAX_COLS * 2
        );
    }

    char *last_line =
        (char *)(VIDEO_ADDRESS + get_screen_offset(0, MAX_ROWS - 1));

    for (i = 0; i < MAX_COLS * 2; i++)
    {
        last_line[i] = 0;
    }


    cursor_offset -= MAX_COLS * 2;

    return cursor_offset;
}


void print_char(
    char character,
    int col,
    int row,
    char attribute_byte
)
{
    unsigned char *vidmem =
        (unsigned char *)VIDEO_ADDRESS;

    int offset;

    if (col >= 0 && row >= 0)
    {
        offset = get_screen_offset(col, row);
    }
    else
    {
        offset = get_cursor();
    }


    if (attribute_byte == 0)
    {
        attribute_byte = WHITE_ON_BLACK;
    }

    if (character == '\n')
    {
        int row = offset / (MAX_COLS * 2);

        offset = get_screen_offset(MAX_COLS - 1, row);
    }
    else
    {
        vidmem[offset] = character;
        vidmem[offset + 1] = attribute_byte;
    }


    offset += 2;

    offset = handle_scrolling(offset);

    set_cursor(offset);
}


void print_at(char *message, int col, int row)
{

    if (col >= 0 && row >= 0)
    {
        set_cursor(get_screen_offset(col, row));
    }

    int i = 0;

    while (message[i] != '\0')
    {
        print_char(message[i], -1, -1, WHITE_ON_BLACK);
        i++;
    }
}

void print_at_color(char *message, int col, int row, char color)
{
    if (col >= 0 && row >= 0)
    {
        set_cursor(get_screen_offset(col, row));
    }

    int i = 0;

    while (message[i] != '\0')
    {
        print_char(message[i], -1, -1, color);
        i++;
    }
}

void print(char *message)
{
    print_at(message, -1, -1);
}


void clear_screen(void)
{
    int row;
    int col;

    for (row = 0; row < MAX_ROWS; row++)
    {
        for (col = 0; col < MAX_COLS; col++)
        {
            print_char(
                ' ',
                col,
                row,
                WHITE_ON_BLACK
            );
        }
    }

    set_cursor(get_screen_offset(0, 0));
}