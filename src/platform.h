
#ifndef PLATFORM_H
#define PLATFORM_H

#define CONSOLE_NEWLINE_N 0
#define CONSOLE_NEWLINE_R_N 1

#ifndef UART_RX_BUFFER_LENGTH
#define UART_RX_BUFFER_LENGTH 60
#endif /* !UART_RX_BUFFER_LENGTH */

#ifndef UART_TX_BUFFER_LENGTH
#define UART_TX_BUFFER_LENGTH 0
#endif /* !UART_TX_BUFFER_LENGTH */

#if UART_TX_BUFFER_LENGTH == 0
#define UART_NO_TX_BUFFER
#endif /* !UART_TX_BUFFER_LENGTH */

/* = Display Stuff = */

#define MAIN_PID 1

#define UART_MID_BASE 100

#define UART_BUFFER_SZ 10

#define QD_PLATFORM_COLINUX   1
#define QD_PLATFORM_X86       2
#define QD_PLATFORM_MSP430    3

#if QD_PLATFORM == QD_PLATFORM_X86 || QD_PLATFORM == QD_PLATFORM_COLINUX

#define DISPLAY_WIDTH 80
#define DISPLAY_HEIGHT 40
#define DISPLAY_INDEX_MAX (DISPLAY_WIDTH * DISPLAY_HEIGHT)

#elif QD_PLATFORM == QD_PLATFORM_MSP430
#include "msp430/platform.h"
#else
#warning "Invalid platform specified!"
#endif /* QD_PLATFORM */

#endif /* PLATFORM_H */

