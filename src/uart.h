
#ifndef UART_H
#define UART_H

#include "platform.h"

#ifndef UART_BAUD_RATE
#define UART_BAUD_RATE  9600
#endif /* !UART_BAUD_RATE */

#ifndef UART_RX_BUFFER_LENGTH
#define UART_RX_BUFFER_LENGTH 60
#endif /* !UART_RX_BUFFER_LENGTH */

#ifndef UART_TX_BUFFER_LENGTH
#define UART_TX_BUFFER_LENGTH 20
#endif /* !UART_TX_BUFFER_LENGTH */

#define UART_PRESENT  0x1
#define _UART_2_PRESENT  0x2
#define _UART_3_PRESENT  0x4
#define _UART_4_PRESENT  0x8
#define UART_SOFT     0xf
#define _UART_2_SOFT     0x20
#define _UART_3_SOFT     0x40
#define _UART_4_SOFT     0x80

uint8_t uart_init();
void uart_clear();
char uart_getc();
uint8_t uart_hit();
void uart_putc( const char c );
void uart_soft_pause();
void uart_soft_resume();

#endif /* UART_H */

