
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"

#define REPL_LINE_SIZE_MAX 20
#define REPL_COMMAND_MAX_LEN 3
#define REPL_COMMANDS_MAX 5

struct repl_command {
   char command[REPL_COMMAND_MAX_LEN];
   void (*callback)();
};

void tregcmd( struct repl_command* );
void tprintf( const char* pattern, ... );
void tputs( const char* str );
TASK_RETVAL trepl_task( TASK_PID pid );

#ifdef CONSOLE_C
struct repl_command g_repl_commands[REPL_COMMANDS_MAX];
#else
extern struct repl_command g_repl_commands[REPL_COMMANDS_MAX];
#endif /* CONSOLE_C */

#endif /* CONSOLE_H */

