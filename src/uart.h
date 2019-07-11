
#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#ifndef UART_RX_BUFFER_DISABLED
#define MISPOS_RING_BUFFER
#endif /* UART_RX_BUFFER_DISABLED */

#define UART_BAUD_RATE  9600

#ifndef UART_RX_BUFFER_LENGTH
#define UART_RX_BUFFER_LENGTH 60
#endif /* UART_RX_BUFFER_LENGTH */

#ifndef UART_TX_BUFFER_LENGTH
#define UART_TX_BUFFER_LENGTH 20
#endif /* UART_TX_BUFFER_LENGTH */

#define UART_INDEX_ALL 0

#define UART_PRESENT  0x1
#define _UART_2_PRESENT  0x2
#define _UART_3_PRESENT  0x4
#define _UART_4_PRESENT  0x8
#define UART_SOFT     0xf
#define _UART_2_SOFT     0x20
#define _UART_3_SOFT     0x40
#define _UART_4_SOFT     0x80

#define UART_MAX_COUNT 4

#ifdef UART_IGNORE_LF
#define UART_NEW_LINE '\r'
#define UART_ANTI_NEW_LINE '\n'
#elif defined( UART_IGNORE_CR )
#define UART_NEW_LINE '\n'
#define UART_ANTI_NEW_LINE '\r'
#else
#define UART_NEW_LINE '\n'
/*
#warning "UART newline automatically set to \\n."
*/
#endif

void uart_init_all();
uint8_t uart_init( uint8_t index );
void uart_clear( uint8_t index );
/* The UART gets chars as unsigned, so they must stay this way. */
char uart_getc( uint8_t dev_index, bool wait );
uint8_t uart_hit( uint8_t dev_index );
void uart_putc( uint8_t dev_index, const char c );
void uart_soft_pause( uint8_t index );
void uart_soft_resume( uint8_t index );

#endif /* UART_H */

