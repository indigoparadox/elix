
#include "adhd.h"

#include "mem.h"

#include <stddef.h>

struct adhd_var {
   enum adhd_var_type type;
   char name[ADHD_VAR_NAME_SIZE];
   int size;
   uint8_t value[];
};

struct adhd_task {
   unsigned long period;
   unsigned long elapsed;
   int (*callback)( int );
};

static struct adhd_task g_tasks[ADHD_TASKS_MAX];
static int g_tasks_len = 0;
static int g_var_counts[ADHD_TASKS_MAX];
static struct adhd_var g_vars[ADHD_VAR_COUNT_GLOBAL_MAX];

void adhd_init() {
   mzero( g_var_counts, ADHD_TASKS_MAX );
   mzero( g_vars, ADHD_VAR_COUNT_GLOBAL_MAX * sizeof( struct adhd_var ) );
}

int adhd_add_task( int (*callback)( int ) ) {
   struct adhd_task* task = NULL;

   if( ADHD_TASKS_MAX <= g_tasks_len + 1 ) {
      /* Too many tasks already! */
      return -1;
   }

   mzero( &(g_tasks[g_tasks_len]), sizeof( struct adhd_task ) );
   task = &(g_tasks[g_tasks_len]);
   task->callback = callback;

   //mcopy( &(g_tasks[tasks_len]), task, sizeof( struct adhd_task ) );

   /* Increment task count, return new task index. */
   g_tasks_len++;
   return g_tasks_len - 1;
}

int adhd_get_tasks_len() {
   return g_tasks_len;
}

int adhd_call_task( int idx ) {
   if( 0 > idx || idx >= g_tasks_len ) {
      /* Invalid task index. */
      return -1;
   }
   return g_tasks[idx].callback( idx );
}

int adhd_get_pid_var_idx( int pid ) {
   int i = 0;

   if( 0 > pid || pid >= g_tasks_len ) {
      /* Invalid task index. */
      return -1;
   }

   /* TODO: Push other vars out of the way, keep task vars together. */
   for( i = 0 ; pid > i ; i++ ) {
      pid_first_var_idx += g_var_counts[i];
   }

   return pid_first_var_idx;
}

void* adhd_get_ptr( int pid, char* name ) {
   int pid_first_var_idx = 0;

   pid_first_var_idx = adhd_get_pid_var_idx( pid );
   if( 0 > pid_first_var_idx ) {
      /* Invalid PID. */
      return;
   }
}

void adhd_set_ptr( int pid, char* name, void* ptr ) {
   int pid_var_idx = 0;

   pid_first_var_idx = adhd_get_pid_var_idx( pid );
   if( 0 > pid_first_var_idx ) {
      /* Invalid PID. */
      return;
   }

   /* Account for other vars belonging to this process. */
   pid_var_idx += g_var_counts[pid];
   if( pid_var_idx > ADHD_VAR_COUNT_GLOBAL_MAX ) {
      /* TODO: Register failure. */
      return;
   }

   g_vars[pid_var_idx].type
   
}

