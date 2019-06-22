
#include "../config.h"

#ifdef DRIVER_UART_INTERNAL

#ifndef UART_H
#define UART_H

#ifndef UART_RX_BUFFER_DISABLED
#define MISPOS_RING_BUFFER
#endif /* UART_RX_BUFFER_DISABLED */

#include "../console.h"
#include "../mispos.h"
#include "../irqal.h"

#if !defined( UCA0BR0_ ) && !defined( UCA0BR0 )
#define DRIVER_UART_SOFT
#endif /* UCA0BR0 */

#if defined( DRIVER_UART_SOFT ) && defined( DRIVER_PWM )
#error "Unable to use soft UART when PWM driver is enabled!"
#endif /* DRIVER_PWM */

#if defined( DRIVER_UART_SOFT ) && !defined( DRIVER_CRYSTAL )
#error "Unable to use soft UART when crystal driver is not enabled!"
#endif /* DRIVER_CRYSTAL */

#include "../cpu.h"

#define UART_BAUD_RATE  9600
#define UART_BIT_CYCLES (CPU_MHZ / UART_BAUD_RATE)
#define UART_BIT_CYCLES_DIV2 (CPU_MHZ / (UART_BAUD_RATE * 2))

#ifndef UART_RX_BUFFER_LENGTH
#define UART_RX_BUFFER_LENGTH 60
#endif /* UART_RX_BUFFER_LENGTH */

#ifndef UART_TX_BUFFER_LENGTH
#define UART_TX_BUFFER_LENGTH 20
#endif /* UART_TX_BUFFER_LENGTH */

#ifndef UART_COUNT
#define UART_COUNT 1
#endif /* UART_COUNT */

#define UART_INDEX_ALL 0

#define UART_ENABLED

/* These are pretty much defined by hardware characteristics, so they can't	*
 * be overridden by config.																	*/
#ifdef DRIVER_UART_SOFT
#define UART_TX BIT1
#define UART_RX BIT2
#else
#define UART_TX BIT2
#define UART_RX BIT1
#endif /* DRIVER_UART_SOFT */
#define UART_PORT P1

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

/* The index of the first software UART. */
/* In a multi-UART system, UART 0 is always hardware. */
#ifdef DRIVER_UART_SOFT
#define UART_SOFT_START_IDX 0
#else
#define UART_SOFT_START_IDX 1
#endif /* DRIVER_UART_SOFT */

/* If we're using a hard UART, then pause/resume are NOOPS. */
#ifndef DRIVER_UART_SOFT
#define uart_soft_pause( index )
#define uart_soft_resume( index )
#else
#define uart_init( index ) uart_soft_resume( index )
#endif /* DRIVER_UART_SOFT */

#ifndef DRIVER_UART_SOFT
uint8_t uart_init( uint8_t index );
#endif /* DRIVER_UART_SOFT */
void uart_clear( uint8_t index );
/* The UART gets chars as unsigned, so they must stay this way. */
unsigned char uart_getc( uint8_t index );
#ifdef UART_GETS
BOOL uart_gets( uint8_t index, char* buffer, uint8_t length );
#endif /* UART_GETS */
void uart_putc( uint8_t index, const char c );
void uart_puts( uint8_t index, const char *str, uint8_t len );
void uart_putn( uint8_t index, uint16_t num, uint8_t base );
#ifdef DRIVER_UART_SOFT
void uart_soft_pause( uint8_t index );
void uart_soft_resume( uint8_t index );
#endif /* DRIVER_UART_SOFT */

#endif /* UART_H */

#else /* DRIVER_UART_INTERNAL */

#define uart_soft_pause()
#define uart_soft_resume()

#endif /* DRIVER_UART_INTERNAL */

