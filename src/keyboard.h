
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_shutdown();
char keyboard_getc( uint8_t dev_index, bool wait );

#endif /* KEYBOARD_H */

