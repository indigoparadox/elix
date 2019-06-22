
#ifndef COMMANDS_H
#define COMMANDS_H

#include "alpha.h"
#include "adhd.h"

typedef TASK_RETVAL (*CONSOLE_CMD)( const struct astring* );

#define CMD_MAX_LEN 10

struct command {
   char command[CMD_MAX_LEN];
   CONSOLE_CMD callback;
} __attribute__( (packed) );

TASK_RETVAL do_command( const struct astring* cli );

#endif /* COMMANDS_H */

