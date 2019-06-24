
#include "../uart.h"

#include "../ring.h"
#include "../alpha.h"
#include "../io.h"
#include "../platform.h"

#include <msp430.h>

//#define UART_BIT_CYCLES ((QD_CPU_MHZ * 1000000) / UART_BAUD_RATE)
//#define UART_BIT_CYCLES_DIV2 ((QD_CPU_MHZ * 1000000) / (UART_BAUD_RATE * 2))

#define uart_is_sw( dev_index ) (dev_index != 0)

#ifdef UART_RX_BUFFER_DISABLED
static unsigned char uart_rx_buffer[UART_COUNT] = { '\0' };
#else
static unsigned char rx_buffer_array[UART_COUNT][UART_RX_BUFFER_LENGTH];
struct ring_buffer rx_buffer_info[UART_COUNT];
#endif /* UART_RX_BUFFER_DISABLED */

#if defined( QD_UART_SW ) || defined( QD_UART_HW )
static bool uart_process_rx( uint8_t index, unsigned char c ) {
#ifdef UART_ANTI_NEW_LINE
	if( UART_ANTI_NEW_LINE == c ) {
		/* Skip cleanup, we don't want to wake up. */
      return FALSE;
	}
#endif /* UART_ANTI_NEW_LINE */

#ifdef UART_RX_BUFFER_DISABLED
   uart_rx_buffer[index] = c;
#else
   ring_buffer_push( c, &(rx_buffer_info[index]) );
#endif /* UART_RX_BUFFER_DISABLED */

	return true;
}
#endif /* QD_UART_SW || QD_UART_HW */

#ifdef QD_UART_SW
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

#endif /* QD_UART_SW */

#ifdef QD_UART_HW

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR( void ) {
	unsigned char rx_in = UCA0RXBUF;
   uint8_t i = 0;

   for( i = 0 ; UART_MAX_COUNT > i ; i++ ) {
      if( !io_flag( i, UART_READY ) ) {
         continue;
      }
      if( !io_flag( i, UART_READY ) ) {
         continue;
      } else if( uart_process_rx( i, rx_in ) ) {
         /* Wake on exit. */
         __bic_SR_register_on_exit( LPM0_bits );
      }
   }
}

#endif /* QD_UART_HW */

uint8_t uart_init( uint8_t dev_index ) {
	uint8_t retval = 0;

#ifndef UART_RX_BUFFER_DISABLED
   ring_buffer_init(
      &(rx_buffer_info[dev_index]),
		&(rx_buffer_array[dev_index][0]),
		UART_RX_BUFFER_LENGTH
   );
#endif /* UART_RX_BUFFER_DISABLED */

   switch( dev_index ) {
#ifdef QD_UART_SW
   case 0:
      /* #0 is always software if present. */
      /* These don't use all this hardware init stuff. */
      uart_soft_resume( dev_index );
      break;
#endif /* QD_UART_SW */

#ifdef QD_UART_HW
   case 1:
      /* (1) Set state machine to the reset state. */
      UCA0CTL1 |= UCSWRST;

      /* (2) Initialize USCI registers. */
      UCA0CTL1 |= UCSSEL_2;               /* CLK = SMCLK */

      /* Modulation */
#if QD_CPU_MHZ == 1
      UCA0BR0 = 104;
      UCA0BR1 = 0x00;
      UCA0MCTL = UCBRS_1;
#elif QD_CPU_MHZ == 8
      /* TODO */
      UCA0MCTL = UCBRS_3 + UCBRF_0;
#elif QD_CPU_MHZ == 16
      /* TODO */
      UCA0MCTL = UCBRS_3 + UCBRF_0;
#else
#error Invalid CPU clock speed specified!
#endif /* QD_CPU_MHZ */

      /* (4) Clear UCSWRST flag. */
      UCA0CTL1 &= ~UCSWRST; /* Initialize USCI state machine. */

      /* (3) Configure ports. */
#ifndef P3SEL
      P1SEL |= UART_1_RX | UART_1_TX;
      P1SEL2 |= UART_1_RX | UART_1_TX;
#else /* P3SEL */
      P3SEL |= BIT3 | BIT4;
#endif /* P3SEL */

#ifdef IE2_
      IE2 |= UCA0RXIE; /* Enable USCI_A0 RX interrupt. */
#else /* IE2_ */
      UCA0IE |= UCTXIE;
#endif /* IE2_ */

      break;

   case 2:
      /* TODO */
      break;
#endif /* QD_UART_HW */
   }

   /* Enable the STATUS_UART_READY bit, even if not using soft UART. */
   io_flag_on( dev_index, UART_READY );

	return retval;
}

#ifdef UART_BUFFER_CLEARABLE
void uart_clear() {
	rx_buffer_index_start = rx_buffer_index_end;
}
#endif /* UART_BUFFER_CLEARABLE */

uint8_t uart_hit( uint8_t dev_index ) {
   return 0; /* TODO */
}

unsigned char uart_getc( uint8_t dev_index ) {
#ifdef UART_RX_BUFFER_DISABLED
   unsigned char out;
   while( '\0' == uart_rx_buffer[dev_index] ) {
      irqal_call_handlers();
      /* Suspend. */
      __bis_SR_register( GIE + LPM0_bits )
   }
   out = uart_rx_buffer[dev_index];
   uart_rx_buffer[dev_index] = '\0';
   return out;
#else
   /* Wait until an actual character is present. */
   ring_buffer_wait( &(rx_buffer_info[dev_index]) );

   return (unsigned char)ring_buffer_pop( &(rx_buffer_info[dev_index]) );
#endif /* UART_RX_BUFFER_DISABLED */
}

void uart_putc( uint8_t dev_index, const char c ) {

   /* Detect if UART is paused. */
   if( !io_flag( dev_index, UART_READY ) ) {
      //return;
   }

   switch( dev_index ) {
#ifdef QD_UART_SW
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

#ifdef QD_UART_HW
   case 1:
#ifdef IFG2_
      while( !(IFG2 & UCA0TXIFG) );
#else /* IFG2_ */
      while( !(UCA0IFG & UCTXIFG) );
#endif /* IFG2_ */
      UCA0TXBUF = c;
      break;

   case 2:
      /* TODO */
      break;
#endif /* QD_UART_HW */
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

