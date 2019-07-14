
#include <msp430.h>

/* = UART Stuff = */

#if !defined( UCA0BR0_ ) && !defined( UCA0BR0 )
#define QD_UART_SW
#endif /* UCA0BR0 */

#if defined( QD_UART_SW ) && defined( DRIVER_PWM )
#error "Unable to use soft UART when PWM driver is enabled!"
#endif /* DRIVER_PWM */

#if defined( QD_UART_SW ) && !defined( DRIVER_CRYSTAL )
#error "Unable to use soft UART when crystal driver is not enabled!"
#endif /* DRIVER_CRYSTAL */

#define CONSOLE_NEWLINE CONSOLE_NEWLINE_R_N

/* These are pretty much defined by hardware characteristics, so they can't	*
 * be overridden by config.																	*/
#define UART_0_TX BIT1
#define UART_0_RX BIT2
#define UART_1_TX BIT2
#define UART_1_RX BIT1

/* = WDT = */

#include "wdt.h"

