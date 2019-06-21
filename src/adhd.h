
#ifndef ADHD_H
#define ADHD_H

#include <stdint.h>
#include <stddef.h>

#include "mem.h"
#include "alpha.h"

#define ADHD_PID_MAIN 1
#define ADHD_PID_FIRST 1

#define ADHD_MID_ENV_MAIN 1
#define ADHD_MID_ENV_SCHEDULER 2
#define ADHD_MID_ENV_CURRENT 3

#define ADHD_TASKS_MAX 5

typedef uint8_t TASK_PID;
typedef uint8_t TASK_RETVAL;
typedef TASK_RETVAL (*ADHD_TASK)();

struct adhd_task;

#define RETVAL_OK 0
#define RETVAL_INVALID_PID 0
#define RETVAL_YIELD 254

#ifdef SCHEDULE_COOP

#define TASK_STATUS_RUN 1
#define TASK_STATUS_EXIT 0

#else

#define RETVAL_KILL 255

#endif /* SCHEDULE_COOP */

struct adhd_task {
   /*unsigned long period;
   unsigned long elapsed;*/
#ifdef SCHEDULE_COOP
   jmp_buf  env;
   uint8_t status;
#endif /* SCHEDULE_COOP */
   TASK_PID pid;
   const struct astring* gid;
   ADHD_TASK callback;
   struct adhd_task* next;
};

#define adhd_get_pid() \
   (g_curr_env->pid)

#define adhd_set_gid( str ) g_curr_env->gid = str

#ifdef SCHEDULE_COOP

#include <setjmp.h>

#define adhd_yield() \
   if( !setjmp( g_curr_env->env ) ) { \
      longjmp( g_sched_env->env, 1 ); \
   }

/** \brief Setup the task struct for a task function. This should be called 
  *        from within the task function before doing anything else.
  */
#define adhd_task_setup() \
   adhd_new_task(); \
   if( !setjmp( g_curr_env->env ) ) { \
      longjmp( g_main_env->env, 1 ); \
   }

#define adhd_launch_task( callback ) \
   if( !setjmp( g_main_env->env ) ) { \
      (callback)(); \
   }

#define adhd_start() \
   g_main_env = mget( \
      ADHD_PID_MAIN, ADHD_MID_ENV_MAIN, sizeof( struct adhd_task ) ); \
   g_sched_env = mget( \
      ADHD_PID_MAIN, ADHD_MID_ENV_SCHEDULER, sizeof( struct adhd_task ) ); \
   if( NULL != g_main_env && NULL != g_sched_env ) { \
      if( !setjmp( g_main_env->env ) ) { \
         adhd_step(); \
      } \
      longjmp( g_sched_env->env, 1 ); \
   }

#define adhd_continue_loop() continue

#define adhd_end_loop() }

#define adhd_exit_task() g_curr_env->status = TASK_STATUS_EXIT

#else /* !SCHEDULE_COOP */

#define adhd_yield() \
   return RETVAL_YIELD

#define adhd_task_setup()

#define adhd_start()

#define adhd_continue_loop() return RETVAL_OK

#define adhd_end_loop()

#define adhd_exit_task() return RETVAL_KILL

void adhd_launch_task( ADHD_TASK callback );
TASK_RETVAL adhd_call_task( TASK_PID pid );
#endif /* SCHEDULE_COOP */

TASK_PID adhd_get_pid_by_gid( const char* gid );
void adhd_step();
struct adhd_task* adhd_new_task();
void adhd_kill_task( TASK_PID pid );
void adhd_wait( BITFIELD status, BITFIELD condition );

#ifdef ADHD_C

#ifdef SCHEDULE_COOP
struct adhd_task* g_main_env = NULL;
#endif /* SCHEDULE_COOP */
struct adhd_task* g_sched_env = NULL;
struct adhd_task* g_curr_env = NULL;

#else

#ifdef SCHEDULE_COOP
extern struct adhd_task* g_main_env;
#endif /* SCHEDULE_COOP */
extern struct adhd_task* g_sched_env;
extern struct adhd_task* g_curr_env;

#endif /* ADHD_C */

#endif /* ADHD_H */

