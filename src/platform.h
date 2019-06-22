
#ifndef PLATFORM_H
#define PLATFORM_H

/* = Display Stuff = */

#define QD_PLATFORM_COLINUX   1
#define QD_PLATFORM_X86       2
#define QD_PLATFORM_MSP430    3

#if QD_PLATFORM == QD_PLATFORM_X86 || QD_PLATFORM == QD_PLATFORM_COLINUX

#define DISPLAY_WIDTH 80
#define DISPLAY_HEIGHT 40
#define DISPLAY_INDEX_MAX (DISPLAY_WIDTH * DISPLAY_HEIGHT)

#elif QD_PLATFORM == QD_PLATFORM_MSP430

/* = UART Stuff = */

#if !defined( UCA0BR0_ ) && !defined( UCA0BR0 )
#define DRIVER_UART_SOFT
#endif /* UCA0BR0 */

#if defined( QD_UART_SW ) && defined( DRIVER_PWM )
#error "Unable to use soft UART when PWM driver is enabled!"
#endif /* DRIVER_PWM */

#if defined( QD_UART_SW ) && !defined( DRIVER_CRYSTAL )
#error "Unable to use soft UART when crystal driver is not enabled!"
#endif /* DRIVER_CRYSTAL */

/* These are pretty much defined by hardware characteristics, so they can't	*
 * be overridden by config.																	*/
#ifdef QD_UART_SW
#define UART_TX BIT1
#define UART_RX BIT2
#else
#define UART_TX BIT2
#define UART_RX BIT1
#endif /* DRIVER_UART_SOFT */
#define UART_PORT P1

/* CPU stuff. */

#if QD_CPU_MHZ == 1

#define CPU_BCS CALBC1_1MHZ
#define CPU_DCO CALDCO_1MHZ

#elif QD_CPU_MHZ == 8

#ifndef DRIVER_CRYSTAL
/* Use pre-calibrated values. */
#define CPU_BCS CALBC1_8MHZ
#define CPU_DCO CALDCO_8MHZ
#endif /* DRIVER_CRYSTAL */

#elif QD_CPU_MHZ == 16

#ifndef DRIVER_CRYSTAL
/* Use pre-calibrated values. */
#define CPU_BCS CALBC1_16MHZ
#define CPU_DCO CALDCO_16MHZ
#endif /* DRIVER_CRYSTAL */

#else
#error "Invalid CPU speed specified!"
#endif /* QD_CPU_MHZ */

#define MICROSECOND CPU_SPEED

#if !defined( DRIVER_CRYSTAL ) && defined( BCSCTL1_ )
#define cpu_init() \
	BCSCTL1 = CPU_BCS; \
	DCOCTL = CPU_DCO;
#elif defined( DRIVER_CRYSTAL )
#define cpu_init()
#else
#define cpu_init()
#endif /* DRIVER_CRYSTAL */

#else

#warning "Invalid platform specified!"

#endif /* QD_PLATFORM */

#endif /* PLATFORM_H */

