
#ifndef CONSOLE_H
#define CONSOLE_H

#include "etypes.h"

#include <stdarg.h>

#include "alpha.h"
#include "kernel.h"
#include "platform.h"

#ifndef QD_CONSOLE_IN_IDX
#define QD_CONSOLE_IN_IDX 0
#endif /* QD_CONSOLE_IN_IDX */

#ifndef QD_CONSOLE_OUT_IDX
#define QD_CONSOLE_OUT_IDX 0
#endif /* QD_CONSOLE_OUT_IDX */

#ifndef CONSOLE_NEWLINE
#define CONSOLE_NEWLINE "\n"
#endif /* CONSOLE_NEWLINE */

#define CONSOLE_CMDS_MAX 5

#define console_const( id, str ) astring_const( id, str )

#define tputs( str ) tprintf( "%a", str )

void tputc( char c );
char tgetc();
char twaitc();
void tprintf( const char* pattern, ... );

#ifdef CONSOLE_C
uint8_t g_console_in_dev_index = QD_CONSOLE_IN_IDX;
uint8_t g_console_out_dev_index = QD_CONSOLE_OUT_IDX;
TASK_PID g_console_pid = 0;
CONSOLE_CMD g_console_apps[CONSOLE_CMDS_MAX] = { 0 };
uint8_t g_console_apps_top = 0;
#else
extern uint8_t g_console_in_dev_index;
extern uint8_t g_console_out_dev_index;
extern TASK_PID g_console_pid;
extern CONSOLE_CMD g_console_apps[CONSOLE_CMDS_MAX];
extern uint8_t g_console_apps_top;
#endif /* CONSOLE_C */

#endif /* CONSOLE_H */

