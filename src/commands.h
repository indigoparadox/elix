
#ifndef COMMANDS_H
#define COMMANDS_H

#include "alpha.h"

typedef int (*CONSOLE_CMD)( char* );

struct command {
   struct astring command;
   int (*callback)( char* cli );
};

#define cmd_def( cmd, callback ) \
   { astring_l( cmd ), callback }

static const struct command g_commands[] = {
   cmd_def( "service", trepl_service ),
   cmd_def( "", NULL )
};

#endif /* COMMANDS_H */

