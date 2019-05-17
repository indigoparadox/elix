
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"

#define REPL_LINE_SIZE_MAX 20
#define REPL_COMMANDS_MAX 5

struct repl_command {
   struct astring* command;
   void (*callback)();
};

void tregcmd( struct repl_command* );
void tprintf( const struct astring* pattern, ... );
void tputs( const struct astring* str );
TASK_RETVAL trepl_task( TASK_PID pid );

#ifdef CONSOLE_C
struct repl_command g_repl_commands[REPL_COMMANDS_MAX];
#else
extern struct repl_command g_repl_commands[REPL_COMMANDS_MAX];
#endif /* CONSOLE_C */

#endif /* CONSOLE_H */

