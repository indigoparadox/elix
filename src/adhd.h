
#ifndef ADHD_H
#define ADHD_H

#define ADHD_TASKS_MAX 5
#define ADHD_VAR_NAME_SIZE 24
#define ADHD_VAR_SIZE_GLOBAL_MAX 20

enum adhd_var_type {
   VAR_TYPE_INT8,
   VAR_TYPE_STR,
   VAR_TYPE_PTR
};

union adhd_var_val {
   int integer;
   char string[ADHD_VAR_VAL_SIZE_MAX];
   void* pointer;
};

struct adhd_task;

int adhd_add_task( int (*callback)( int ) );
int adhd_get_tasks_len();
int adhd_call_task( int idx );
void* adhd_get_ptr( int pid, char* name );
void adhd_set_ptr( int pid, char* name, void* ptr );

#endif /* ADHD_H */

