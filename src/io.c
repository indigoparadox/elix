
#include "io.h"

io_putc_cb putc_devs[IO_DEVS_MAX];
uint8_t putc_devs_top;
io_getc_cb getc_devs[IO_DEVS_MAX];
uint8_t getc_devs_top;

void io_regoutdev( io_putc_cb cb ) {
   putc_devs[putc_devs_top++] = cb;
}

void io_regindev( io_getc_cb cb ) {
   getc_devs[getc_devs_top++] = cb;
}

