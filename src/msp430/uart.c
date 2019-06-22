
#include "uart.h"

#ifdef DRIVER_UART_INTERNAL

system_status_enable();

#ifdef UART_RX_BUFFER_DISABLED
static unsigned char uart_rx_buffer[UART_COUNT] = { '\0' };
#else
static unsigned char rx_buffer_array[UART_COUNT][UART_RX_BUFFER_LENGTH];
struct mispos_ring_buffer rx_buffer_info[UART_COUNT];
#endif /* UART_RX_BUFFER_DISABLED */

static BOOL uart_process_rx( uint8_t index, unsigned char c ) {
#ifdef UART_ANTI_NEW_LINE
	if( UART_ANTI_NEW_LINE == c ) {
		/* Skip cleanup, we don't want to wake up. */
      return FALSE;
	}
#endif /* UART_ANTI_NEW_LINE */

#ifdef UART_RX_BUFFER_DISABLED
   uart_rx_buffer[index] = c;
#else
   mispos_ring_buffer_push( c, &(rx_buffer_info[index]) );
#endif /* UART_RX_BUFFER_DISABLED */

#ifndef UART_IGNORE_NEWLINE_FLAG
	if( UART_NEW_LINE == c ) {
		/* TODO: Scale for multiple indexes. */
		system_status_on( STATUS_UART_NEWLINE );
	}
#endif /* UART_IGNORE_NEWLINE_FLAG */

	return TRUE;
}

#if defined( DRIVER_UART_SOFT ) || defined( DRIVER_UART_MULTI )

static volatile uint16_t uart_tx_buffer[UART_COUNT] = { 0 };
static volatile uint8_t uart_tx_bit_count[] = {[0 ... UART_COUNT] = 10 }; /* Includes encapsulation. */

#pragma vector = TIMERA0_VECTOR
__interrupt void SOFTUARTTX_ISR( void ) {
	/* TODO: Scale for multiple indexes. */
#ifdef DRIVER_UART_MULTI
	uint8_t index;

	for( index = UART_SOFT_START_IDX ; UART_COUNT > index ; index++ ) {
#else
   uint8_t index = 0;
#endif /* DRIVER_UART_MULTI */
      if( !system_status( STATUS_UART_READY ) ) {
         return;
      } else if( 0 == uart_tx_bit_count[index] ) {
         /* Done transmitting, so shut off and clean up. */
         TACCTL0 &= ~CCIE;
         uart_tx_bit_count[index] = 10;
         system_status_off( STATUS_PWM_TA0 );
      } else {
         /* Transmit the next bit and then shove it off the end. */
         TACCR0 += UART_BIT_CYCLES;
         if( 0x01 & uart_tx_buffer[index] ) {
            TACCTL0 &= ~OUTMOD2;
         } else {
            TACCTL0 |= OUTMOD2;
         }
         uart_tx_buffer[index] >>= 1;
         uart_tx_bit_count[index]--;
      }
#ifdef DRIVER_UART_MULTI
	}
#endif /* DRIVER_UART_MULTI */
}

static volatile uint8_t uart_rx_bit_count[] = {[0 ... UART_COUNT] = 8 };
static unsigned char uart_rx_c[UART_COUNT] = { 0 };

#pragma vector = TIMERA1_VECTOR
__interrupt void SOFTUARTRX_ISR( void ) {
	uint8_t index;

#ifdef DRIVER_UART_MULTI
	/* In a multi-UART system, UART 0 is always hardware. */
	for( index = 1 ; UART_COUNT > index ; index++ ) {
#elif defined( DRIVER_UART_SOFT )
	/* If using a single soft UART, replace hardware UART 0. */
	index = 0; 
#endif /* DRIVER_UART_MULTI */
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
#ifdef DRIVER_UART_MULTI
	}
#endif /* DRIVER_UART_MULTI */
}

#else /* DRIVER_UART_SOFT */

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR( void ) {
	unsigned char rx_in = UCA0RXBUF;

   if( !system_status( STATUS_UART_READY ) ) {
      return;
	/* TODO: Scale for multiple indexes. */
	} else if( uart_process_rx( 0, rx_in ) ) {
      mispos_wake_on_exit();
	}
}

#endif /* DRIVER_UART_SOFT */

#ifndef DRIVER_UART_SOFT

uint8_t uart_init( uint8_t index ) {
	uint8_t retval = 0;

#ifndef UART_RX_BUFFER_DISABLED
   mispos_ring_buffer_init(
      &(rx_buffer_info[index]),
		&(rx_buffer_array[index][0]),
		UART_RX_BUFFER_LENGTH
   );
#endif /* UART_RX_BUFFER_DISABLED */

	/* Setup the hardware UART. */

	/* (1) Set state machine to the reset state. */
   UCA0CTL1 = UCSWRST;

   /* (2) Initialize USCI registers. */
   UCA0CTL1 |= UCSSEL_2;               /* CLK = SMCLK */

	/* We get some large numbers above, so we have to truncate them. */
	UCA0BR0 = (uint8_t)UART_BIT_CYCLES;
	UCA0BR1 = (uint8_t)(UART_BIT_CYCLES >> 8);                     

#ifdef CPU_CLOCK_1MHZ
	/* Modulation UCBRSx = 1 */
	UCA0MCTL = UCBRS0;
#elif defined( CPU_CLOCK_8MHZ )
	UCA0MCTL = UCBRS_3 + UCBRF_0;
#elif defined( CPU_CLOCK_16MHZ )
	/* TODO */
	UCA0MCTL = UCBRS_3 + UCBRF_0;
#endif /* CPU_CLOCK */

   /* (3) Configure ports. */
#ifndef P3SEL
   P1SEL |= mispos_bits_or( UART_RX, UART_TX );
   P1SEL2 |= mispos_bits_or( UART_RX, UART_TX );
#else /* P3SEL */
   P3SEL |= BIT3 | BIT4;
#endif /* P3SEL */

cleanup:

   /* (4) Clear UCSWRST flag. */
   UCA0CTL1 &= ~UCSWRST;               /* Initialize USCI state machine. */

	if( 0 == retval ) {
#ifdef IE2_
		IE2 |= UCA0RXIE;                    /* Enable USCI_A0 RX interrupt. */
#else /* IE2_ */
      UCA0IE |= UCTXIE;
#endif /* IE2_ */
	}

   /* Enable the STATUS_UART_READY bit, even if not using soft UART. */
   system_status_on( STATUS_UART_READY );

	return retval;
}

#endif /* DRIVER_UART_SOFT */

#ifdef UART_BUFFER_CLEARABLE
void uart_clear( uint8_t index ) {
	rx_buffer_index_start = rx_buffer_index_end;
}
#endif /* UART_BUFFER_CLEARABLE */

unsigned char uart_getc( uint8_t index ) {
#if 0
#ifdef DRIVER_UART_SOFT
   if( !system_status( STATUS_UART_READY ) ) {
      /* Detect if UART is paused. */
      return '\0';
	} else 
#endif /* DRIVER_UART_SOFT */
#endif

#ifdef UART_RX_BUFFER_DISABLED
   unsigned char out;
   while( '\0' == uart_rx_buffer[index] ) {
      irqal_call_handlers();
      mispos_suspend();
   }
   out = uart_rx_buffer[index];
   uart_rx_buffer[index] = '\0';
   return out;
#else
   /* Wait until an actual character is present. */
   mispos_ring_buffer_wait( &(rx_buffer_info[index]) );

   return (unsigned char)mispos_ring_buffer_pop( &(rx_buffer_info[index]) );
#endif /* UART_RX_BUFFER_DISABLED */
}

#ifdef UART_GETS
BOOL uart_gets( char* buffer, uint8_t length ) {
	uint8_t i = 0;
	char c;
	BOOL retval = TRUE;

	while( length > i ) {
		c = uart_getc();
		if( 0 == i && ('\0' == c || UART_NEW_LINE == c) ) {
#ifndef UART_IGNORE_NEWLINE_FLAG
			system_status_off( STATUS_UART_NEWLINE );
#endif /* UART_IGNORE_NEWLINE_FLAG */
			while( length > i ) {
				buffer[i] = '\0';
				i++;
			}
			retval = FALSE;
			goto cleanup;
		} else if( UART_NEW_LINE == c ) {
#ifndef UART_IGNORE_NEWLINE_FLAG
			system_status_off( STATUS_UART_NEWLINE );
#endif /* UART_IGNORE_NEWLINE_FLAG */
			while( length > i ) {
				buffer[i] = '\0';
				i++;
			}
			goto cleanup;
		}

		buffer[i] = c;
		i++;
	}

cleanup:

	return retval;
}
#endif /* UART_GETS */

void uart_putc( uint8_t index, const char c ) {

#ifdef DRIVER_UART_SOFT
   /* Detect if UART is paused. */
	/* TODO: Scale for multiple indexes. */
   if( !system_status( STATUS_UART_READY ) ) {
      return;
   }
#endif /* DRIVER_UART_SOFT */

#ifdef DRIVER_UART_SOFT

   while( TACCTL0 & CCIE ) {
      asm( ";" ); /* Don't optimize away. */
   }

#ifdef UART_SOFT_BLINK_TX
   led_1_blink( 200 );
#endif /* UART_SOFT_BLINK_TX */

   uart_tx_buffer[index] = c;

	/* Add encapsulation bits. */
   uart_tx_buffer[index] |= 0x100;
   uart_tx_buffer[index] <<= 1;

	/* Ready timer to transmit new byte. */
   TA0CCR0 = TAR;
   TA0CCR0 += UART_BIT_CYCLES;
   TA0CCTL0 = OUTMOD0 + CCIE;

#else /* DRIVER_UART_SOFT */

#ifdef IFG2_
	while( !(IFG2 & UCA0TXIFG) );
#else /* IFG2_ */
	while( !(UCA0IFG & UCTXIFG) );
#endif /* IFG2_ */
  	UCA0TXBUF = c;

#endif /* DRIVER_UART_SOFT */
}

/*
 * Parameters:
 *    - index: Index of UART to print to.
 *    - str: Address of string to print.
 *    - len: Number of chars to print at max.
 */
void uart_puts( uint8_t index, const char *str, uint8_t len ) {
	int i = 0;

#ifdef BOUNDS_CHECK_DISABLED
   while( '\0' != str[i] ) {
#else
   while( '\0' != str[i] && len > i ) {
#endif /* BOUNDS_CHECK_DISABLED */

#ifdef UART_NEW_LINE_DOUBLE_OUT
		if( '\n' == str[i] ) {
			uart_putc( index, '\r' );
		}
#endif /* UART_NEW_LINE_DOUBLE_OUT */
 		uart_putc( index, str[i++] );
	}
   return;
}

void uart_putn( uint8_t index, uint16_t num, uint8_t base ) {
	char str[UINT16_DIGITS];
	mispos_utoa( num, str, UINT16_DIGITS, base );
	uart_puts( index, str, UINT16_DIGITS );
}

#ifdef DRIVER_UART_SOFT

void uart_soft_pause( uint8_t index ) {
   TACCR0 = 0;
   TACCTL0 = 0;
   TACCTL1 = 0;
   TACTL = 0;
   system_status_off( STATUS_PWM_TA0 );

   /* Only ever pause the soft UART. No reason to pause hard UART. */
   system_status_off( STATUS_UART_READY );
}

void uart_soft_resume( uint8_t index ) {
   mispos_lock_wait( STATUS_PWM_TA0, TRUE );

   system_status_on( STATUS_PWM_TA0 );

	mispos_reg( UART_PORT, OUT ) |= mispos_bits_or( UART_TX, UART_RX );
	mispos_reg( UART_PORT, SEL ) |= mispos_bits_plus( UART_TX, UART_RX );
	mispos_reg( UART_PORT, DIR ) |= mispos_bits_or( UART_TX );

	/* UART has the timer allll to itself, for now. */
	TACCTL0 = OUT;
   TACCTL1 = SCS + CM1 + CAP + CCIE;
   TACTL = TASSEL_2 + MC_2 + TACLR;

   system_status_on( STATUS_UART_READY );

#ifdef UART_PRIME_U
   if( !system_status( STATUS_UART_READY ) ) {
      /* U is 01010101, good to establish the connection. */
      /* XXX: This will cause a stack overflow. */
      uart_putc( index, 'U' );
   }
#endif /* UART_PRIME_U */

}

#endif /* DRIVER_UART_SOFT */

#endif /* DRIVER_UART_INTERNAL */

