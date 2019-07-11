
#ifndef IRQAL_H
#define IRQAL_H

#include <stdint.h>

#ifndef IRQAL_HANDLERS_COUNT_MAX
#define IRQAL_HANDLERS_COUNT_MAX 5
#endif /* IRQAL_HANDLERS_COUNT_MAX */

#ifndef IRQAL_HANDLER_REGS_COUNT_MAX
#define IRQAL_HANDLER_REGS_COUNT_MAX 3
#endif /* IRQAL_HANDLER_REGS_COUNT_MAX */

struct irqal_handler;

typedef void (*IRQAL_CALLBACK)( struct irqal_handler* );
typedef uint8_t IRQALTYPE_T;

#define IRQAL_TYPE_NONE     0
#define IRQAL_TYPE_WDT      1
#define IRQAL_TYPE_PORT_1   2
#define IRQAL_TYPE_ADC      3

void irqal_handler_deregister( uint8_t index );
int8_t irqal_add_handler(
	IRQALTYPE_T type, IRQAL_CALLBACK callback, uint8_t repeat );
void irqal_call_handlers( IRQALTYPE_T type );

#endif /* IRQAL_H */

