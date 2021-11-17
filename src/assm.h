
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
   /*! \brief Unresolved labels that should be resolved by a last pass with
    *         assm_resolve_labels()
    */
   struct ASSM_LABEL* unresolved;
   /*! \brief Current byte index from the start of the output file. */
   unsigned short ipc;
   /*! \brief Modifiers to the current state (e.g. ::ASSM_FLAG_ESCAPE for
    *         ::STATE_STRING or ::STATE_CHAR.
    */
   unsigned char flags;
   /*! \brief Byte array output bytecode is written to. */
   unsigned char* out_buffer;
   /*! \brief Where the next output byte should be (over)written in
    *         ASSM_STATE::out_buffer.
    */
   int out_buffer_pos;
   /*! \brief Number of output bytes allocated to ASSM_STATE::out_buffer. */
   int out_buffer_sz;
   /*! \brief The number of bytes in use out of ASSM_STATE::out_buffer_sz in
    *         ASSM_STATE::out_buffer.
    */
   int out_buffer_len;
   /*! \brief The current section being assembled. */
   int section;
   int section_offset;
   /*! \brief The current instruction if the state is ::STATE_PARAMS. */
   int instr;
   /*! \brief The current state of the parser. */
   int state;
   /*! \brief The next index to use for a ::STATE_ALLOC. */
   unsigned char next_alloc_mid;
};

#define global_set_flags( flag, global ) \
   assm_dprintf( 1, "set global flag: %d", flag ); \
   global->flags |= flag;

#define global_unset_flags( flag, global ) \
   assm_dprintf( 1, "unset global flag: %d", flag ); \
   global->flags &= ~flag;

/*! \brief ASSM_STATE::flags indicating the next character is special. */
#define ASSM_FLAG_ESCAPE   0x01

#define ASSM_MASK_DOUBLE   0x80
#define ASSM_MASK_OP       0x7f

/*! \brief No particular state. Build a token and get ready for a sigil
 *         indicating its use.
 */
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
 *
 * Allocs are just variables used by the assembler during assembly for values
 * that can be hard-coded into the executable so long as they're consistant
 * (e.g. memory block indexes).
 *
 * They're handled using the label mechanism but ignored by the last-pass
 * resolver as they're always resolved on first reference in the first pass
 * anyway.
 */
#define STATE_ALLOC     7
/*! \brief Inside of a label token. */
#define STATE_LABEL     8
/*! \brief SYSC is a special case OP with a parameter token that resolves. */
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

/*! \brief Mapping of op tokens to match with their opcodes by index. */
extern const char* gc_vm_op_tokens[];
extern const uint8_t gc_vm_op_argcs[];
/*! \brief Mapping of state codes to printable strings for debugging. */
extern const char* gc_assm_states[];
/*! \brief Mapping of sysc to match with their sysc by index. */
extern const char* gc_sysc_tokens[];

#endif /* ASSM_C */

#ifndef DEBUG_THRESHOLD
#define DEBUG_THRESHOLD 0
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

/**
 * \brief Clear the current global token appended to by the processor.
 * \param global The current assembler state.
 */
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
/**
 * \brief Make a last pass to resolve unresolved labels to their offsets.
 * \param global The current assembler state.
 * \return Number of labels resolved or -1 on failure.
 */
int assm_resolve_labels( struct ASSM_STATE* global );

#endif /* ASSM_H */

