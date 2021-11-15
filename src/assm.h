
#ifndef ASSM_H
#define ASSM_H

/*! \file assm.h
 *  \brief Bytecode assembler for creating user applications.
 */

#include "vm.h"
#include "sysc.h"

/*! \brief Maximum size of token names. */
#define ASSM_TOKEN_MAX 255
#define ASSM_OUT_BUFFER_INITIAL_SZ 1024

struct ASSM_LABEL {
   /*! \brief Byte index of the label from the start of the file. */
   unsigned short ipc;
   char name[ASSM_TOKEN_MAX];
   struct ASSM_LABEL* next;
};

struct ASSM_STATE {
   char token[ASSM_TOKEN_MAX];
   struct ASSM_LABEL* labels;
   struct ASSM_LABEL* unresolved;
   /*! \brief Current byte index from the start of the output file. */
   unsigned short ipc;
   unsigned char flags;
   unsigned char* out_buffer;
   int out_buffer_pos;
   int out_buffer_sz;
   int out_buffer_len;
   int section;
   int instr;
   int state;
   unsigned char next_alloc_mid;
};

#define global_set_flags( flag, global ) \
   assm_dprintf( 1, "set global flag: %d", flag ); \
   global->flags |= flag;

#define global_unset_flags( flag, global ) \
   assm_dprintf( 1, "unset global flag: %d", flag ); \
   global->flags &= ~flag;

#define ASSM_FLAG_ESCAPE 1

#define ASSM_MASK_DOUBLE   0x80
#define ASSM_MASK_OP       0x7f

#define STATE_NONE      0
#define STATE_SECTION   1
/*! \brief Looking for parameters to previous op. */
#define STATE_PARAMS    2
/*! \brief Inside of a char literal. */
#define STATE_CHAR      3
/*! \brief Inside of a string literal. */
#define STATE_STRING    4
/**
 * \brief Inside of an integer immediate.
 *
 * 
 */
#define STATE_NUM       5
/**
 * \brief Inside of a comment.
 *
 * Will end when newline is detected.
 */
#define STATE_COMMENT   6
/**
 * \brief Inside of an alloc (memory block index).
 */
#define STATE_ALLOC     7
#define STATE_LABEL     8
#define STATE_SYSC      9

#ifdef ASSM_C

/* === If we're being called inside assm.c === */

#define VM_OP_STR_LIST( idx, argc, op, token ) token,

const char* gc_vm_op_tokens[] = {
   VM_OP_TABLE( VM_OP_STR_LIST )
   "" /* Terminator for easier looping. */
};

#define VM_OP_ARGC_LIST( idx, argc, op, token ) argc,

const uint8_t gc_vm_op_argcs[] = {
   VM_OP_TABLE( VM_OP_ARGC_LIST )
};

const char* gc_assm_states[] = {
   "none",
   "section",
   "params",
   "char",
   "string",
   "num",
   "comment",
   "alloc",
   "label",
   "sysc"
};

#define SYSC_STR_LIST( op, token ) token,

const char* gc_sysc_tokens[] = {
   SYSC_TABLE( SYSC_STR_LIST )
   "" /* Terminator for easier looping. */
};

#else

extern const char* gc_vm_op_tokens[];
extern const uint8_t gc_vm_op_argcs[];
extern const char* gc_assm_states[];
extern const char* gc_sysc_tokens[];

#endif /* ASSM_C */

#ifndef DEBUG_THRESHOLD
#define DEBUG_THRESHOLD 1
#endif /* !DEBUG_THRESHOLD */

#define assm_dprintf( lvl, ... ) \
   if( lvl >= DEBUG_THRESHOLD ) { \
      printf( "(%d) " __FILE__ ": %d: ", lvl, __LINE__ ); \
      printf( __VA_ARGS__ ); \
      printf( "\n" ); \
      fflush( stdout ); \
   }

#define assm_eprintf( ... ) \
   fprintf( stderr, "(E) " __FILE__ ": %d: ", __LINE__ ); \
   fprintf( stderr, __VA_ARGS__ ); \
   fprintf( stderr, "\n" ); \
   fflush( stderr ); \

void reset_token( struct ASSM_STATE* global );
unsigned short get_label_ipc(
   char* label, unsigned short ipc_of_call, struct ASSM_STATE* global );
void write_bin_instr_or_data( unsigned char c, struct ASSM_STATE* global );
void write_double_bin_instr_or_data(
   unsigned short u, struct ASSM_STATE* global );
void set_global_instr( int instr_bytecode, struct ASSM_STATE* global );
void set_global_state( int state, struct ASSM_STATE* global );
void append_to_token( char c, struct ASSM_STATE* global );
void append_to_string( char c, struct ASSM_STATE* global );
unsigned char token_to_sysc( char* token );
void process_char( char c, struct ASSM_STATE* global );
void process_token( struct ASSM_STATE* global );

#endif /* ASSM_H */

