
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init();
void keyboard_shutdown();
int keyboard_hit( uint8_t dev_index );
char keyboard_getc( uint8_t dev_index );

#endif /* KEYBOARD_H */

