
#ifndef REPL_H
#define REPL_H

#include "../adhd.h"
#include "../kernel.h"
#include "../console.h"

/* Memory IDs for repl tasks. */
#define REPL_MID_LINE      101
#define REPL_MID_CUR_POS   102
#define REPL_MID_FLAGS     103

#define REPL_LINE_SIZE_MAX 20

#define REPL_FLAG_INITIALIZED    0x01
#define REPL_FLAG_ANSI_SEQ       0x02

TASK_RETVAL trepl_task();
const char* trepl_tok( struct astring* cli, uint8_t idx );
TASK_RETVAL repl_command( struct astring* cli, TASK_PID repl_pid );

#endif /* REPL_H */

