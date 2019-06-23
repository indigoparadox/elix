
#ifndef WDT_H
#define WDT_H

#include <stdint.h>

#define WDT_CALLS_PER_MICROSECOND (CPU_SPEED * 2) /* 2 calls per ms. */

#ifdef QD_WDT_DISABLED

#define wdt_enable()
#define wdt_get_counter()

#else /* QD_WDT_DISABLED */

#define wdt_enable() extern volatile uint32_t wdt_counter;
#define wdt_get_counter() wdt_counter

#endif /* QD_WDT_DISABLED */

void wdt_init( void );

#ifndef WDT_SLEEP_DISABLED
void wdt_sleep( uint16_t microseconds );
#endif /* WDT_SLEEP_DISABLED */

#endif /* WDT_H */

