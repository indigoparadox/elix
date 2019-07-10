
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stddef.h>

#include "platform.h"

/* These are defines and not an enum, as enum args may be > 8 bits. */

#define COLOR_BLACK           0x00
#define COLOR_BLUE            0x01
#define COLOR_GREEN           0x02
#define COLOR_CYAN            0x03
#define COLOR_RED             0x04
#define COLOR_MAGENTA         0x05
#define COLOR_BROWN           0x06
#define COLOR_LIGHT_GREY      0x07
#define COLOR_DARK_GREY       0x08
#define COLOR_LIGHT_BLUE      0x09
#define COLOR_LIGHT_GREEN     0x0a
#define COLOR_LIGHT_CYAN      0x0b
#define COLOR_LIGHT_RED       0x0c
#define COLOR_LIGHT_MAGENTA   0x0d
#define COLOR_LIGHT_BROWN     0x0e
#define COLOR_WHITE           0x0f

#define display_newline( dev_index ) display_putc( dev_index, '\n' )

void display_set_colors( uint8_t fg, uint8_t bg );
void display_shutdown();
void display_putc_at( char c, int x, int y );
void display_putc( uint8_t dev_index, char c );

#endif /* DISPLAY_H */
 
