
#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>

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

#define IO_CBS_MAX 5

typedef void (*OUTPUT_CB)( uint8_t, char );
typedef char (*INPUT_CB)( uint8_t, bool );

#ifdef IO_C
volatile uint8_t g_io_flags;
INPUT_CB g_io_input_cbs[IO_CBS_MAX];
uint8_t g_io_input_idx = 0;
OUTPUT_CB g_io_output_cbs[IO_CBS_MAX];
uint8_t g_io_output_idx = 0;
#else
extern uint8_t g_io_flags;
extern INPUT_CB g_io_input_cbs[IO_CBS_MAX];
extern uint8_t g_io_input_idx;
extern OUTPUT_CB g_io_output_cbs[IO_CBS_MAX];
extern uint8_t g_io_output_idx;
#endif /* IO_C */

void io_call_handlers();

#endif /* IO_H */

