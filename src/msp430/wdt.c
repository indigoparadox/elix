
#include "wdt.h"

#include <msp430.h>

#ifndef QD_WDT_DISABLED

//volatile register uint32_t wdt_counter asm( "r6" );
volatile uint32_t wdt_counter;
static volatile uint8_t wdt_counter_interval = 0;

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR( void ) {
	if( 0 >= wdt_counter_interval-- ) {
		wdt_counter += 1;
		wdt_counter_interval = WDT_CALLS_PER_MICROSECOND;
		mispos_wake_on_exit();
	}
}

#ifndef WDT_SLEEP_DISABLED

void sleep_handler( uint32_t count, struct irqal_handler_instance* handler ) {
	/* system_status_off( STATUS_WDT_SLEEPING ); */
}

void wdt_sleep( uint16_t microseconds ) {
	system_status_on( STATUS_WDT_SLEEPING );
	irqal_add_handler(
		IRQAL_HANDLER_TYPE_WDT,
		sleep_handler,
		wdt_get_counter() + microseconds,
		0
	);
	mispos_lock_wait( STATUS_WDT_SLEEPING, TRUE );
}

#endif /* WDT_SLEEP_DISABLED */

#endif /* QD_WDT_DISABLED */

void wdt_init( void ) {
#if QD_WDT_DISABLED

   WDTCTL = WDTPW + WDTHOLD;

#else /* QD_WDT_DISABLED */

WDTCTL = WDT_MDLY_0_5;
   wdt_counter = 0;
#ifdef IE1_
	IE1 |= WDTIE;
#else /* IE1_ */
   SFRIE1 |= WDTIE;
#endif /* IE1_ */

#endif /* QD_WDT_DISABLED */
}

