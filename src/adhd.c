
#include "code16.h"

#define ADHD_C
#include "adhd.h"
#include "cstd.h"

#if 0
#ifdef __GNUC__
const
#endif /* __GNUC__ */
struct astring g_str_none = astring_l( "none" );
#endif

static struct adhd_task* g_tasks;

void adhd_start() {
   /* TODO: Free this in shutdown. */
   g_tasks = malloc( sizeof( struct adhd_task ) * ADHD_TASKS_MAX );
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

   memset( &(g_tasks[pid_iter]), '\0', sizeof( struct adhd_task ) );
   task = &(g_tasks[pid_iter]);
   task->pid = pid_iter;
   task->callback = callback;
   memcpy( task->gid, gid, 4 );

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

   /* TODO: Call task cleanup. */

   /* TODO: Go through memory and remove any allocated blocks for this PID. */
   //mfree_all( pid );
   
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
         0 == strncmp( gid, g_tasks[i].gid, 4 )
      ) {
         return i;
      }
   }

   return TASK_PID_INVALID;
}

