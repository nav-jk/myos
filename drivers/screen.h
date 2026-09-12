#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xB8000

#define MAX_ROWS 25
#define MAX_COLS 80

#define WHITE_ON_BLACK 0x0F

/* VGA controller ports */
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

void print_char(char character, int col, int row, char attribute_byte);
void print_at(char *message, int col, int row);
void print_at_color(char *message, int col, int row, char color);
void print(char *message);
void clear_screen(void);

#endif