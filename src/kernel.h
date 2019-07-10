
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#include "adhd.h"
#include "alpha.h"
#include "keyboard.h"
#include "console.h"

#define STDIN 0

#define VERSION "2019.21"

#define SYSTEM_RUNNING 0
#define SYSTEM_SHUTDOWN 1

#define KERNEL_MID_CLI 5

#define CMD_MAX_LEN 10

#define CTOR_PRIO_DISPLAY 105
#define CTOR_PRIO_UART 110

typedef TASK_RETVAL (*CONSOLE_CMD)( const struct astring* );

struct api_command {
   char command[CMD_MAX_LEN];
   CONSOLE_CMD callback;
} __attribute__( (packed) );

#ifdef USE_EXT_CLI
int kmain( int argc, char** argv );
#else
int kmain();
#endif /* USE_EXT_CLI */

#ifdef KERNEL_C
uint8_t g_system_state = SYSTEM_RUNNING;
#else
extern uint8_t g_system_state;
#endif /* KERNEL_C */

#endif /* KERNEL_H */

