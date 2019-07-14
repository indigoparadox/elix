
#include "platform.h"

#include "../kernel.h"
#include "../uart.h"
#include "../console.h"

#ifdef DRIVER_CRYSTAL
static void crystal_set_dco( unsigned int delta ) {
	uint16_t compare,
		old_capture = 0;

   /* Wait for crystal to stabilize. */
	for( compare = 0 ; 0xfffe > compare ; compare++ );

	BCSCTL1 |= DIVA_3;
	TA0CCTL0 = CM_1 + CCIS_1 + CAP;
	TA0CTL = TASSEL_2 + MC_2 + TACLR;

	while( 1 ){
		while( !(CCIFG & TA0CCTL0) );
		TA0CCTL0 &= ~CCIFG;
		compare = TACCR0;
		compare -= old_capture;
		old_capture = TA0CCR0;

		if( delta == compare ) {
			break;
		} else if( delta < compare ) {
			DCOCTL--;
			if( DCOCTL == 0xFF && BCSCTL1 & 0x0f ) {
            BCSCTL1--;
			}
		} else {
			DCOCTL++;
			if( DCOCTL == 0x00 && (BCSCTL1 & 0x0f) != 0x0f ) {
            BCSCTL1++;
			}
		}
	}

	TA0CCTL0 = 0;
	TA0CTL = 0;
	BCSCTL1 &= ~DIVA_3;
}
#endif /* DRIVER_CRYSTAL */

int main() {

	/* wdt_init(); */
   //WDTCTL = WDTPW + WDTHOLD;

   DCOCTL = 0;
#if QD_CPU_MHZ == 1
   BCSCTL1  =  CALBC1_1MHZ;
   DCOCTL   =  CALDCO_1MHZ;
#elif QD_CPU_MHZ == 8
   BCSCTL1  =  CALBC1_8MHZ;
   DCOCTL   =  CALDCO_8MHZ;
#elif QD_CPU_MHZ == 12
   BCSCTL1  =  CALBC1_12MHZ;
   DCOCTL   =  CALDCO_12MHZ;
#elif QD_CPU_MHZ == 16
   BCSCTL1  =  CALBC1_16MHZ;
   DCOCTL   =  CALDCO_16MHZ;
#endif /* QD_CPU_MHZ */

#if 0
   while( 1 ) {
      //UCA0TXBUF = 'c';
      uart_putc( 1, 'c' );
   }
#endif


#if 0
#ifdef DRIVER_CRYSTAL
	crystal_set_dco( (QD_CPU_MHZ * 1000000) / 4096 );
#else
   /* Use pre-calibrated values. */
   DCOCTL = 0;
#if QD_CPU_MHZ == 1
   BCSCTL1 = CALBC1_1MHZ;
   DCOCTL = CALDCO_1MHZ;
#elif QD_CPU_MHZ == 8
   BCSCTL1 = CALBC1_8MHZ;
   DCOCTL = CALDCO_8MHZ;
#elif QD_CPU_MHZ == 16
   BCSCTL1 = CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;
#else
#error "Invalid CPU speed specified!"
#endif /* QD_CPU_MHZ */

#endif /* DRIVER_CRYSTAL */

#if 0
   P1DIR = BIT0; /* XXX */
   //P1OUT &= ~BIT6;
   P1OUT = BIT0;
#endif
#endif

   __enable_interrupt();

   kmain();
   return 0;
}

