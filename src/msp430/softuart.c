
#include "../uart.h"

static volatile uint16_t uart_tx_buffer[UART_COUNT] = { 0 };
static volatile uint8_t uart_tx_bit_count[] = {[0 ... UART_COUNT] = 10 }; /* Includes encapsulation. */

#pragma vector = TIMERA0_VECTOR
__interrupt void SOFTUARTTX_ISR( void ) {
	uint8_t i = 0;

   for( i = 0 ; UART_MAX_COUNT > i ; i++ ) {
      if( !io_flag( i, UART_READY ) && !uart_is_sw( i ) ) {
         /* Hardware UART doesn't need to use PWM. */
         return;
      } else if( 0 == uart_tx_bit_count[i] ) {
         /* Done transmitting, so shut off and clean up. */
         TACCTL0 &= ~CCIE;
         uart_tx_bit_count[i] = 10;
         io_flag_off( 0, PWM_ON ); /* Soft UART always uses PWM1. */
      } else {
         /* Transmit the next bit and then shove it off the end. */
         TACCR0 += UART_BIT_CYCLES;
         if( 0x01 & uart_tx_buffer[i] ) {
            TACCTL0 &= ~OUTMOD2;
         } else {
            TACCTL0 |= OUTMOD2;
         }
         uart_tx_buffer[i] >>= 1;
         uart_tx_bit_count[i]--;
      }
   }
}

static volatile uint8_t uart_rx_bit_count[] = {[0 ... UART_COUNT] = 8 };
static unsigned char uart_rx_c[UART_COUNT] = { 0 };

#pragma vector = TIMERA1_VECTOR
__interrupt void SOFTUARTRX_ISR( void ) {
	const uint8_t index = 0; /* Soft UART is always index 0. */

   if( TA0IV == TA0IV_TACCR1 ) {
    	TACCR1 += UART_BIT_CYCLES;
      if( TACCTL1 & CAP ) {
      	TACCTL1 &= ~CAP;
         TACCR1 += UART_BIT_CYCLES_DIV2;
      } else {
         uart_rx_c[index] >>= 1;
         if( TACCTL1 & SCCI ) {
         	uart_rx_c[index] |= 0x80;
         }
         uart_rx_bit_count[index]--;
         if( 0 == uart_rx_bit_count[index] ) {
				/* Store the complete byte and reset for the next one. */
				uart_process_rx( index, uart_rx_c[index] );
				uart_rx_bit_count[index] = 8;
            TACCTL1 |= CAP;
            TACCR1 += UART_BIT_CYCLES;
         }
      }
   }
}

uint8_t uart_init( uint8_t dev_index ) {
   switch( dev_index ) {
   case 0:
      /* #0 is always software if present. */
      /* These don't use all this hardware init stuff. */
      uart_soft_resume( dev_index );
      break;
   }

   io_flag_on( dev_index, UART_READY );
}

void uart_putc( uint8_t dev_index, const char c ) {
   if( !io_flag( dev_index, UART_READY ) ) {
      return;
   }

   switch( dev_index ) {
   case 0:
      /* UART 0 is always software if present. */

      while( TACCTL0 & CCIE ) {
         asm( ";" ); /* Don't optimize away. */
      }

#ifdef UART_SW_BLINK_TX
      /* led_1_blink( 200 ); */
#endif /* UART_SOFT_BLINK_TX */

      uart_tx_buffer[dev_index] = c;

      /* Add encapsulation bits. */
      uart_tx_buffer[dev_index] |= 0x100;
      uart_tx_buffer[dev_index] <<= 1;

      /* Ready timer to transmit new byte. */
      TA0CCR0 = TAR;
      TA0CCR0 += UART_BIT_CYCLES;
      TA0CCTL0 = OUTMOD0 + CCIE;
#endif /* QD_UART_SW */
   }
}

void uart_soft_pause( uint8_t index ) {
#ifdef QD_UART_SW
   TACCR0 = 0;
   TACCTL0 = 0;
   TACCTL1 = 0;
   TACTL = 0;
   io_flag_off( 0, PWM_ON ); /* Turn off PWM used to simulate UART. */

   /* Only ever pause the soft UART. No reason to pause hard UART. */
   
   io_flag_off( index, UART_READY );
#endif /* QD_UART_SW */
}

#ifdef QD_UART_SW
static void uart_lock_wait( uint8_t dev_index, uint8_t flag, bool is_true ) {
   while(
		(is_true && io_flag( dev_index, flag )) ||
		(!is_true && !io_flag( dev_index, flag ))
	) {
      /* Keep calling handlers or the status may never go away. */
      //io_call_handlers();
      /* Otherwise, suspend and wait for IRQ. */
      /* TODO: Multitask. */
      __bis_SR_register( GIE + LPM0_bits );
   }
}
#endif /* QD_UART_SW */

void uart_soft_resume( uint8_t index ) {
#ifdef QD_UART_SW
   /* Wait for PWM timer to finish. */
   uart_lock_wait( 0, BIT1, true );

   io_flag_on( 0, PWM_ON );

	P1OUT |= UART_0_TX | UART_RX;
	P1SEL |= UART_0_TX | UART_RX;
	P1DIR |= UART_0_TX;

	/* UART has the timer allll to itself, for now. */
	TACCTL0 = OUT;
   TACCTL1 = SCS + CM1 + CAP + CCIE;
   TACTL = TASSEL_2 + MC_2 + TACLR;

   io_flag_on( index, UART_READY );

#ifdef UART_PRIME_U
   if( !io_flag( index, UART_READY ) ) {
      /* U is 01010101, good to establish the connection. */
      /* XXX: This will cause a stack overflow. */
      uart_putc( index, 'U' );
   }
#endif /* UART_PRIME_U */

#endif /* QD_UART_SW */
}

