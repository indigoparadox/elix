
#include <stddef.h>

#include "../irqal.h"
#include "../mem.h"

struct irqal_handler {
   IRQAL_CALLBACK callback;
	uint8_t type;
	int8_t repeat;
};

static struct irqal_handler g_irqal_handlers[IRQAL_HANDLERS_COUNT_MAX];

__attribute__((critical))
void irqal_handler_deregister( uint8_t handler_index ) {
	mzero( &(g_irqal_handlers[handler_index]), sizeof( struct irqal_handler ) );
}

__attribute__((critical))
uint8_t irqal_add_handler(
   IRQALTYPE_T type, IRQAL_CALLBACK callback, int8_t repeat
) {
   uint8_t i = 0;

   for( i = 0 ; IRQAL_HANDLERS_COUNT_MAX > i ; i++ ) {
      if( IRQAL_TYPE_NONE == g_irqal_handlers[i].type ) {
         g_irqal_handlers[i].repeat = repeat;
         g_irqal_handlers[i].callback = callback;
         g_irqal_handlers[i].type = type;
         break;
      }
   }

	return i;
}

/* This should only be called from critical code. */
void irqal_call_handlers( IRQALTYPE_T type ) {
	uint8_t i;

	for( i = 0 ; IRQAL_HANDLERS_COUNT_MAX > i ; i++ ) {

      if( type != g_irqal_handlers[i].type ) {
         continue;
      }

		g_irqal_handlers[i].callback( i, &(g_irqal_handlers[i]) );

		if( !g_irqal_handlers[i].repeat ) {
			/* Just get rid of it when no repeats left. */
         g_irqal_handlers[i].type = IRQAL_TYPE_NONE;
		}
	}
}

