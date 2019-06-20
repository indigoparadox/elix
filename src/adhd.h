
#ifndef ADHD_H
#define ADHD_H

#include <stdint.h>

#include "mem.h"

#define ADHD_TASKS_MAX 5

typedef uint8_t TASK_PID;
typedef uint8_t TASK_RETVAL;
typedef TASK_RETVAL (*ADHD_TASK)( TASK_PID );

struct adhd_task;

#define RETVAL_OK 0
#define RETVAL_KILL 255

#define adhd_yield( pid ) \
   if( !setjmp( env->env ) ) { \
      longjmp( g_env_adhd, 1 ); \
   }

TASK_PID adhd_add_task( ADHD_TASK callback );
TASK_PID adhd_get_tasks_len();
TASK_RETVAL adhd_call_task( TASK_PID pid );
void adhd_kill_task( TASK_PID pid );
void adhd_wait( BITFIELD status, BITFIELD condition );

#endif /* ADHD_H */

