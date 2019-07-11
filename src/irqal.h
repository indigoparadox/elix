
#ifndef IRQAL_H
#define IRQAL_H

/*! \file irqal.h */

#include <stdint.h>

#ifndef IRQAL_HANDLERS_COUNT_MAX
#define IRQAL_HANDLERS_COUNT_MAX 5
#endif /* IRQAL_HANDLERS_COUNT_MAX */

#ifndef IRQAL_HANDLER_REGS_COUNT_MAX
#define IRQAL_HANDLER_REGS_COUNT_MAX 3
#endif /* IRQAL_HANDLER_REGS_COUNT_MAX */

#define IRQAL_REPEAT_INDEFINITE -1

struct irqal_handler;

typedef void (*IRQAL_CALLBACK)( struct irqal_handler* );
typedef uint8_t IRQALTYPE_T;

#define IRQAL_TYPE_NONE     0
#define IRQAL_TYPE_WDT      1
#define IRQAL_TYPE_PORT_1   2
#define IRQAL_TYPE_ADC      3

/*! \brief  Create a handler for the specified type of event.
 *  @param type      The IRQALTYPE_T indicating for what event to trigger the
 *                   handler.
 *  @param callback  The function to execute when the handler is triggered.
 *  @param repeat    The number of times to repeat the handler.
 *                   IRQAL_REPEAT_INDEFINITE to repeat indefinitely.
 */
int8_t irqal_add_handler(
	IRQALTYPE_T type, IRQAL_CALLBACK callback, int8_t repeat );
void irqal_handler_deregister( uint8_t index );
void irqal_call_handlers( IRQALTYPE_T type );

#endif /* IRQAL_H */

