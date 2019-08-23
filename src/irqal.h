
#ifndef IRQAL_H
#define IRQAL_H

/*! \file irqal.h */

#include "etypes.h"

#ifndef IRQAL_HANDLERS_COUNT_MAX
#define IRQAL_HANDLERS_COUNT_MAX 5
#endif /* IRQAL_HANDLERS_COUNT_MAX */

#define IRQAL_REPEAT_INDEFINITE -1

struct irqal_handler;

typedef void (*IRQAL_CALLBACK)( uint8_t index, struct irqal_handler* );
typedef uint8_t IRQALTYPE_T;

#define IRQAL_TYPE_NONE    0
#define IRQAL_TYPE_WDT     1
#define IRQAL_TYPE_ADC     2
#define IRQAL_TYPE_UART_TX 3
#define IRQAL_TYPE_PORT_1  4

/*! \brief  Create a handler for the specified type of event.
 *  @param type      The IRQALTYPE_T indicating for what event to trigger the
 *                   handler.
 *  @param callback  The function to execute when the handler is triggered.
 *  @param repeat    The number of times to repeat the handler.
 *                   IRQAL_REPEAT_INDEFINITE to repeat indefinitely.
 *  @return The index of the created handler in the global array.
 */
uint8_t irqal_add_handler(
	IRQALTYPE_T type, IRQAL_CALLBACK callback, int8_t repeat );

/*! \brief  Remove the handler with the specified index.
 *  @param index  The index of the handler to remove from the global array.
 */
void irqal_handler_deregister( uint8_t index );

/*! \brief  Execute callbacks for all handlers of a specific event.
 *  @param type   The type of event for which all handlers should be executed.
 */
void irqal_call_handlers( IRQALTYPE_T type );

void irqal_interrupts_off();
void irqal_interrupts_on();

#endif /* IRQAL_H */

