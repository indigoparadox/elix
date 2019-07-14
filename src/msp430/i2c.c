
#include "../i2c.h"

#include "../irqal.h"

void i2c_tx_handler( uint8_t index, struct irqal_handler* handler ) {
   if( IFG2 & UCB0TXIFG ) {
      //IFG2 &= ~UCB0TXIE;
      IE2 &= ~UCB0TXIE;
	}
}

void i2c_init() {
	irqal_add_handler(
		IRQAL_TYPE_UART_TX, i2c_tx_handler, IRQAL_REPEAT_INDEFINITE );

	//IE2 |= UCB0TXIE;
}

__attribute( (critical) )
void i2c_start( uint8_t addr ) {

	if( g_uart_status & UART_STATUS_I2C_READY ) {
		return;
	}
	g_uart_status |= UART_STATUS_I2C_READY;

   /* (1) Set state machine to the reset state. */
	UCB0CTL1 |= UCSWRST;

   /* (2) Initialize USCI registers. */
	UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
	UCB0CTL1 = UCSSEL_2 | UCSWRST;

   /* Modulation */
	UCB0BR0 = 0x40;

	/* Set the slave address. */
	UCB0I2CSA = addr;

	P1SEL |= BIT6 | BIT7;
	P1SEL2 |= BIT6 | BIT7;

   /* (4) Clear UCSWRST flag. */
	UCB0CTL1 &= ~UCSWRST;

	/* Commence transfer. */
	UCB0CTL1 |= UCTR | UCTXSTT;
}

void i2c_stop() {
	if( !(g_uart_status & UART_STATUS_I2C_READY) ) {
		return;
	}
	g_uart_status &= UART_STATUS_I2C_READY;

	/* Shut off the connection. */
	UCB0CTL1 |= UCTXSTP;
	IE2 &= ~UCB0TXIE;
}

void i2c_send_byte( uint8_t byte ) {
	if( !(g_uart_status & UART_STATUS_I2C_READY) ) {
		return;
	}

	/* Wait for previous bytes to go through.
#ifdef IFG2_
	while( !(IFG2 & UCB0TXIFG) );
#else /* IFG2_ */
	while( !(UCB0IFG & UCTXIFG) );
#endif /* IFG2_ */

	/* Send the next byte. */
	UCB0TXBUF = byte;
	IE2 |= UCB0TXIE;
}

