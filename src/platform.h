
#ifndef PLATFORM_H
#define PLATFORM_H

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

