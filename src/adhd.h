
#ifndef ADHD_H
#define ADHD_H

#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>

#include "mem.h"

#define ADHD_PID_MAIN 1
#define ADHD_PID_FIRST 1

#define ADHD_MID_ENV_MAIN 1
#define ADHD_MID_ENV_SCHEDULER 2
#define ADHD_MID_ENV_CURRENT 3

#define ADHD_TASKS_MAX 5

typedef uint8_t TASK_PID;
typedef uint8_t TASK_RETVAL;
typedef TASK_RETVAL (*ADHD_TASK)( TASK_PID );

struct adhd_task;

#define RETVAL_OK 0
#define RETVAL_KILL 255

struct adhd_task {
   /*unsigned long period;
   unsigned long elapsed;*/
   jmp_buf  env;
   TASK_PID pid;
   /* char gid[5]; */ /* 4 chars and 1 NULL. */
   /* TASK_RETVAL (*callback)( TASK_PID ); */
   struct adhd_task* next;
};

#define adhd_yield() \
   if( !setjmp( env->env ) ) { \
      longjmp( g_sched_env->env, 1 ); \
   }

/** \brief Setup the task struct for a task function. This should be called 
  *        from within the task function before doing anything else.
  */
#define adhd_task_setup() \
   struct adhd_task* task; \
   task = adhd_new_task(); \
   if( !setjmp( task->env ) ) { \
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

void adhd_step();
struct adhd_task* adhd_new_task();
void adhd_kill_task( TASK_PID pid );
void adhd_wait( BITFIELD status, BITFIELD condition );

#ifdef ADHD_C
struct adhd_task* g_main_env = NULL;
struct adhd_task* g_sched_env = NULL;
#else
extern struct adhd_task* g_main_env;
extern struct adhd_task* g_sched_env;
#endif /* ADHD_C */

#endif /* ADHD_H */

