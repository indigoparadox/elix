
#include "code16.h"

#define ADHD_C
#include "adhd.h"

#include "alpha.h"

#if 0
#ifdef __GNUC__
const
#endif /* __GNUC__ */
struct astring g_str_none = astring_l( "none" );
#endif

#ifdef SCHEDULE_COOP

static struct adhd_task* g_head_env = NULL;
static uint8_t g_next_pid = ADHD_PID_FIRST;

TASK_PID adhd_get_pid_by_gid( const_char* gid ) {
   /* TODO */
}

void adhd_step() {
   /* Setup the scheduler target and go back to main if successful. */
   if( !setjmp( g_sched_env->env ) ) {
      longjmp( g_main_env->env, 1 );
   }

   /* Scheduler target starts here. */

   /* Jump to the next task. */
   g_curr_env = g_curr_env->next;
   longjmp( g_curr_env->env, 1 );
}

TASK_PID adhd_new_task() {
   struct adhd_task* task = NULL;

   /* Setup the task struct and PID. */
   task = mget( ADHD_PID_MAIN, g_next_pid, sizeof( struct adhd_task ) );
   if( NULL == task ) {
      goto cleanup;
   }

   task->status = TASK_STATUS_RUN;
   task->pid = g_next_pid;
   g_next_pid++;

   if( NULL == g_head_env ) {
      /* Start the ring with this task. */
      g_head_env = task;
      task->next = g_head_env;
      g_curr_env = g_head_env;
   } else {
      /* Insert the task into the ring. */
      task->next = g_curr_env->next;
      g_curr_env->next = task;
      g_curr_env = task;
   }

cleanup:
   return task->pid;
}

#else

static struct adhd_task g_tasks[ADHD_TASKS_MAX];

void adhd_start() {
   mzero( g_tasks, sizeof( struct adhd_task ) * ADHD_TASKS_MAX );
}

TASK_RETVAL adhd_launch_task( ADHD_TASK callback, const char* gid ) {
   struct adhd_task* task = NULL;
   TASK_PID pid_iter = 0;

   /* Check for existing instance. */
   pid_iter = adhd_get_pid_by_gid( gid );
   if( TASK_PID_INVALID != pid_iter ) {
      /* Task exists! */
      return RETVAL_TASK_EXISTS;
   }

   /* Check for next PID. */
   pid_iter = 0;
   while( NULL != g_tasks[pid_iter].callback && ADHD_TASKS_MAX > pid_iter ) {
      pid_iter++;
   }
   if( ADHD_TASKS_MAX <= pid_iter ) {
      /* Too many tasks already! */
      return RETVAL_TOO_MANY_TASKS;
   }

   mzero( &(g_tasks[pid_iter]), sizeof( struct adhd_task ) );
   task = &(g_tasks[pid_iter]);
   task->pid = pid_iter;
   task->callback = callback;
   mcopy( task->gid, gid, 4 );

   return RETVAL_OK;
}

TASK_RETVAL adhd_call_task( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || NULL == g_tasks[pid].callback ) {
      /* Invalid task index. */
      return RETVAL_INVALID_PID;
   }
   g_curr_env = &(g_tasks[pid]);
   return g_tasks[pid].callback();
}

void adhd_kill_task( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || NULL == g_tasks[pid].callback ) {
      /* Invalid task index. */
      return;
   }

   /* TODO: Go through memory and remove any allocated blocks for this PID. */
   mfree_all( pid );
   
   g_tasks[pid].callback = NULL;
}

const_char* adhd_get_gid_by_pid( TASK_PID pid ) {
   TASK_PID i = 0;
   
   for( i = 0 ; ADHD_TASKS_MAX > i ; i++ ) {
      if(
         NULL != g_tasks[i].callback &&
         g_tasks[i].pid == pid
      ) {
         if( NULL == g_tasks[i].gid ) {
            return "none";
         } else {
            return g_tasks[i].gid;
         }
      }
   }

   return NULL;
}

TASK_PID adhd_get_pid_by_gid( const_char* gid ) {
   TASK_PID i = 0;
   
   for( i = 0 ; ADHD_TASKS_MAX > i ; i++ ) {
      if(
         NULL != g_tasks[i].callback &&
         NULL != g_tasks[i].gid &&
         0 == alpha_cmp_cc( gid, 4, g_tasks[i].gid, 4, ' ', true, true )
      ) {
         return i;
      }
   }

   return TASK_PID_INVALID;
}

#endif /* SCHEDULE_COOP */

#if 0
void adhd_wait( BITFIELD status, BITFIELD condition ) {
   //while( test_status( status ) != condition ) {
      /* TODO: Keep calling handlers or the status may never go away. */
      /* TODO: Otherwise, just wait for IRQ. */
   //}
}
#endif

