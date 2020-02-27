
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

/* Internal repl flags are reserved starting at 9th bit. */
#define REPL_FLAG_ANSI_SEQ       0x0100

#define repl_set_flag( flag ) (g_repl_flags |= flag)
#define repl_unset_flag( flag ) (g_repl_flags &= flag)
#define repl_flag( flag ) (g_repl_flags & flag)

TASK_RETVAL trepl_task();
void repl_set_line_handler( CONSOLE_CMD new_handler );
const char* trepl_tok( char* cli, uint8_t idx );
TASK_RETVAL repl_command( char* cli );
TASK_RETVAL launch_repl();

#ifndef REPL_C
extern
#endif /* !REPL_C */
uint16_t g_repl_flags; /* !< Scratch space for the fore console app. */

#endif /* REPL_H */

