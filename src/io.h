
#ifndef IO_H
#define IO_H

#include <stdint.h>

#define IO_DEVS_MAX 8

typedef void (*io_putc_cb)( char c );
typedef char (*io_getc_cb)();

void io_regoutdev( io_putc_cb cb );
void io_regindev( io_getc_cb cb );

#endif /* IO_H */

