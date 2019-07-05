
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define STDIN 0

#define SYSTEM_RUNNING 0
#define SYSTEM_SHUTDOWN 1

#define KERNEL_MID_CLI 5

int kmain( int argc, char** argv );

#ifdef KERNEL_C
uint8_t g_system_state = SYSTEM_RUNNING;
#else
extern uint8_t g_system_state;
#endif /* KERNEL_C */

#endif /* KERNEL_H */

