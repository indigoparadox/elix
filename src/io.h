
#ifndef IO_H
#define IO_H

#include "etypes.h"

/**
 * This module helps to coordinate between different functions (UART/SPI/etc)
 * that may need to share resources (e.g. physical pins).
 */

#define UART_READY    0x01
#define _UART_2_READY    0x02
#define _UART_3_READY    0x04
#define _UART_4_READY    0x08
#define PWM_ON        0x0f
#define _PWM_2_ON        0x20

#ifdef QD_UART_SW
#define io_flag( dev_index, flag ) \
   (g_io_flags & (flag << dev_index))

#define io_flag_on( dev_index, flag ) \
   g_io_flags |= (flag << dev_index)

#define io_flag_off( dev_index, flag ) \
   g_io_flags &= ~(flag << dev_index)
#endif /* QD_UART_SW */

#define io_reg_input_cb( cb ) g_io_input_cbs[g_io_input_idx++] = cb

#define io_reg_output_cb( cb ) g_io_output_cbs[g_io_output_idx++] = cb

#include "globals.h"

#endif /* IO_H */

