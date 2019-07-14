
#define UART_C
#include "../uart.h"

#include "../alpha.h"
#include "../kernel.h"
#include "../io.h"
#include "../platform.h"

/* TEMP */
uint8_t g_uart_init = 0;

#include <msp430.h>

/* Dynamic calculation of USCI register values for a requested clockspeed and
 * baud rate:
 */

/* Calculate USCI_BR0_VAL and USCI_BR1_VAL. */
#define USCI_INPUT_CLK           (QD_CPU_MHZ * 1000000)
#define USCI_DIV_INT             (USCI_INPUT_CLK/UART_BAUD_RATE)
#define USCI_BR0_VAL             (USCI_DIV_INT & 0x00FF)
#define USCI_BR1_VAL             ((USCI_DIV_INT >> 8) & 0xFF)

/* Calculate USCI_BRS_VAL. */
#define USCI_DIV_FRAC_NUMERATOR \
   (USCI_INPUT_CLK - (USCI_DIV_INT*UART_BAUD_RATE))
#define USCI_DIV_FRAC_NUM_X_8    (USCI_DIV_FRAC_NUMERATOR*8)
#define USCI_DIV_FRAC_X_8        (USCI_DIV_FRAC_NUM_X_8/UART_BAUD_RATE)
#if (((USCI_DIV_FRAC_NUM_X_8-(USCI_DIV_FRAC_X_8*UART_BAUD_RATE))*10)/UART_BAUD_RATE < 5)
#define USCI_BRS_VAL             (USCI_DIV_FRAC_X_8<< 1)
#else
#define USCI_BRS_VAL             ((USCI_DIV_FRAC_X_8+1)<< 1)
#endif

/* UART interrupt handlers. */

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR( void ) {
   /* Place incoming characters into the RX buffer. */
   if( IFG2 & UCA0RXIFG ) {
      /* Place the incoming input at the end of the buffer. */
      g_uart_rx_buffer[g_uart_rx_buffer_end++] = UCA0RXBUF;
      if( UART_RX_BUFFER_LENGTH <= g_uart_rx_buffer_end ) {
         g_uart_rx_buffer_end = 0;
      }
      if( g_uart_rx_buffer_end == g_uart_rx_buffer_start ) {
         /* The RC buffer is full, so overwrite the oldest input with the
          * newest.
          */
         g_uart_rx_buffer_start++;
         if( UART_RX_BUFFER_LENGTH <= g_uart_rx_buffer_start ) {
            g_uart_rx_buffer_start = 0;
         }
      }
   } else {
      IFG2 &= ~UCB0RXIFG;
   }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR( void ) {
   if( IFG2 & UCA0TXIFG ) {
      /* Done transmitting, so turn off the ISR. */
      IE2 &= ~UCA0TXIE;
   } else {
      IFG2 &= ~UCB0TXIFG;
   }
}

/* Exposed utility functions. */

//__attribute__( (constructor(CTOR_PRIO_UART)) )
uint8_t uart_init() {
	uint8_t retval = 0;
   uint8_t i = 0;

   if( g_uart_init ) {
      return 0;
   }
   g_uart_init = 1;

   /* Disable UART-related interrupts during init. */
      IE2 &= ~(UCA0TXIE | UCA0RXIE | UCB0TXIE | UCB0RXIE );

   /* (1) Set state machine to the reset state. */
   UCA0CTL1 |= UCSWRST;

   /* (2) Initialize USCI registers. */
   UCA0CTL1 |= UCSSEL_2;               /* CLK = SMCLK */

   /* Modulation */
   UCA0BR0 = USCI_BR0_VAL;
   UCA0BR1 = USCI_BR1_VAL;
   UCA0MCTL = USCI_BRS_VAL;

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
   UCA0IE |= UCRXIE;
#endif /* IE2_ */

   /* Enable the STATUS_UART_READY bit, even if not using soft UART. */
   //io_flag_on( dev_index, UART_READY );

   /* Print out some init characters to grease things along. */
   uart_putc( 'U' );
   uart_putc( 'U' );
   uart_putc( 'U' );
   for( i = 0 ; 30 > i ; i++ ) {
      uart_putc( '.' );
   }
   uart_putc( '\r' );
   uart_putc( '\n' );
   
	return retval;
}

uint8_t uart_hit() {
   /* Is there input waiting in the RX buffer? */
   if( g_uart_rx_buffer_start != g_uart_rx_buffer_end ) {
      P1OUT &= ~BIT0;
      return 1;
   }
   return 0;
}

char uart_getc() {
   char out;

   /* Is there input waiting in the RX buffer? */
   if( g_uart_rx_buffer_start == g_uart_rx_buffer_end ) {
      return '\0';
   }

   /* Grab waiting input from the start of the RX buffer. */
   out = g_uart_rx_buffer[g_uart_rx_buffer_start++];
   if( UART_RX_BUFFER_LENGTH <= g_uart_rx_buffer_start ) {
      /* Wrap around. */
      g_uart_rx_buffer_start = 0;
   }

   return out;
}

void uart_putc( const char c ) {
   /* Wait for current transmission to finish. */
#ifdef IFG2_
   while( !(IFG2 & UCA0TXIFG) );
#else /* IFG2_ */
   while( !(UCA0IFG & UCTXIFG) );
#endif /* IFG2_ */

   /* Place the character into the transmit buffer and turn on the ISR. */
   UCA0TXBUF = c;
   IE2 |= UCA0TXIE;
}

