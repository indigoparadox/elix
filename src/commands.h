
#ifndef COMMANDS_H
#define COMMANDS_H

#include "alpha.h"
#include "adhd.h"

typedef TASK_RETVAL (*CONSOLE_CMD)( const struct astring* );

struct command {
   struct astring command;
   CONSOLE_CMD callback;
};

TASK_RETVAL do_command( const struct astring* cli );

#endif /* COMMANDS_H */

