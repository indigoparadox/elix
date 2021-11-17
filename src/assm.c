
#define ASSM_C
#ifdef ASSM_NO_VM
#endif /* ASSM_NO_VM */
#include "assm.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SZ 40960

void reset_token( struct ASSM_STATE* global ) {
   memset( global->token, '\0', ASSM_TOKEN_MAX );
}

void add_label( char* name, int ipc, struct ASSM_LABEL** labels ) {
   struct ASSM_LABEL* label_iter = *labels;

   if( NULL == *labels ) {
      *labels = calloc( 1, sizeof( struct ASSM_LABEL ) );
      assert( NULL != *labels );
      label_iter = *labels;

   } else {
      while( NULL != label_iter->next ) {
         label_iter = label_iter->next;
      }
      label_iter->next = calloc( 1, sizeof( struct ASSM_LABEL ) );
      assert( NULL != label_iter->next );
      label_iter = label_iter->next;
   }

   strncpy( label_iter->name, name, ASSM_TOKEN_MAX );
   label_iter->ipc = ipc;

   assm_dprintf( 1, "label: %s @ %d%s", name, ipc,
      '$' == name[0] ? " (dynamic)" : "" );
}

unsigned short get_label_ipc(
   char* label, unsigned short ipc_of_call, struct ASSM_STATE* global
) {
   struct ASSM_LABEL* label_iter = global->labels;

   while(
      NULL != label_iter &&
      (0 != strncmp( label_iter->name, label, strlen( label_iter->name ) ) ||
      strlen( label ) != strlen( label_iter->name ))
   ) {
      assm_dprintf( 0, "searching for %s, found %s (%d)...",
         label, label_iter->name, label_iter->ipc );
      label_iter = label_iter->next;
   }
   if( NULL != label_iter ) {
      assm_dprintf( 2, "searching for %s, found %s (%d)...",
         label, label_iter->name, label_iter->ipc );
      assert( 0 == strncmp(
         label_iter->name, label, strlen( label_iter->name ) ) );
      assert( strlen( label ) == strlen( label_iter->name ) );
   }

   if( NULL == label_iter ) {
      if( '$' != label[0] ) {
         /* Not an alloc, so add it to the list to search for in next pass. */
         assm_eprintf( "label not found; added to unresolved list" );
         add_label( label, ipc_of_call, &(global->unresolved) );
      }
      return 0;
   }

   assm_dprintf( 2, "label for %s: %d", label, label_iter->ipc );

   return label_iter->ipc;
}

void write_bin_instr_or_data( unsigned char c, struct ASSM_STATE* global ) {
   unsigned char* new_out_buffer = NULL;

   if( NULL == global->out_buffer ) {
      global->out_buffer_sz = ASSM_OUT_BUFFER_INITIAL_SZ;
      global->out_buffer = calloc( global->out_buffer_sz, 1 );
      assert( NULL != global->out_buffer );
   } else if( global->out_buffer_pos + 1 >= global->out_buffer_sz ) {
      global->out_buffer_sz *= 2;
      new_out_buffer = realloc( global->out_buffer, global->out_buffer_sz );
      assert( NULL != new_out_buffer );
      global->out_buffer = new_out_buffer;
   }
   
   /* fwrite( &c, sizeof( char ), 1, bin_file ); */

   global->out_buffer[global->out_buffer_pos] = c;
   if( global->out_buffer_pos == global->out_buffer_len ) {
      global->out_buffer_len++;
   }
   global->out_buffer_pos++;
   global->ipc;
   assm_dprintf( 1, "wrote %d (0x%x), ipc: %d", c, c, global->ipc );
   global->ipc++;
}

void write_double_bin_instr_or_data(
   unsigned short u, struct ASSM_STATE* global
) {
   assm_dprintf( 1, "first byte:" );
   write_bin_instr_or_data( (uint8_t)((u >> 8) & 0x00ff), global );
   assm_dprintf( 1, "second byte:" );
   write_bin_instr_or_data( (uint8_t)(u & 0x00ff), global );
   assm_dprintf( 1, "previous writes wrote %d (0x%x) (0x%x 0x%x), ipc: %d, %d",
      u,
      u,
      (uint8_t)((u >> 8) & 0x00ff),
      (uint8_t)(u & 0x00ff),
      global->ipc - 1,
      global->ipc);
}

void append_to_token( char c, struct ASSM_STATE* global ) {
   assert( strlen( global->token ) < ASSM_TOKEN_MAX );
   global->token[strlen( global->token )] = c;
}

void append_to_string( char c, struct ASSM_STATE* global ) {
   assm_dprintf( 1, "str c: %c", c );
   write_bin_instr_or_data( c, global );
}

uint8_t token_to_op( char* token ) {
   uint8_t i = 0,
      op_out = 0;

   /* Find the instruction with this token. */
   while( '\0' != gc_vm_op_tokens[i][0] ) {
      if( 0 == strncmp(
         token, gc_vm_op_tokens[i], strlen( gc_vm_op_tokens[i] )
      ) ) {
         op_out = i;
         break;
      }
      i++;
   }

   if( 0 == op_out ) {
      if( 0 < strlen( token ) ) {
         assm_eprintf( "invalid token specified: %s", token );
         assert( 1 == 0 );
      }
      goto cleanup;
   }

   #if 0
   if( 'l' == token[strlen( gc_vm_op_tokens[op_out] )] ) {
      /* Set the high bit to indicate op on a double. */
      op_out |= 0x80;
   }
   #endif

cleanup:

   return op_out;
}

uint8_t token_to_sysc( char* token ) {
   uint8_t i = 0,
      sysc_out = 0;

   while( '\0' != gc_sysc_tokens[i][0] ) {
      if( 0 == strncmp(
         token, gc_sysc_tokens[i], strlen( gc_sysc_tokens[i] )
      ) ) {
         sysc_out = i;
         break;
      }
      i++;
   }

   if( 0 == sysc_out ) {
      assm_eprintf( "invalid token specified: %s\n", token );
      assert( 1 == 0 );
   }

   return sysc_out;
}

void set_global_instr( int instr_bytecode, struct ASSM_STATE* global ) {
   if( 0 == instr_bytecode ) {
      assm_dprintf( 1, "no longer processing instruction" );
   } else {
      assm_dprintf( 1, "processing instruction: %s",
         gc_vm_op_tokens[instr_bytecode & ASSM_MASK_OP] );
   }
   global->instr = instr_bytecode;
}

void set_global_section( int section, struct ASSM_STATE* global ) {
   assm_dprintf( 1, "new section: %d", section );
   global->section = section;
}

void process_token( struct ASSM_STATE* global ) {
   int instr_bytecode = 0;
   unsigned short label_ipc = 0;

   switch( global->state ) {
   case STATE_NONE:
      /* Try to decode token as an instruction. */
      instr_bytecode = token_to_op( global->token );
      if( 0 >= instr_bytecode && 0 > strlen( global->token ) ) {
         assm_eprintf( "unknown instruction: %s", global->token );
         assert( 1 == 0 );
      } else if( 0 == strlen( global->token ) ) {
         /* No token, just whitespace. */
         break;
      }

      assm_dprintf( 2, "token: %s (%d)",
         gc_vm_op_tokens[instr_bytecode & ASSM_MASK_OP], instr_bytecode );
      if( 0 < gc_vm_op_argcs[instr_bytecode & ASSM_MASK_OP] ) {
         /* Tokens that take arguments mean arguments are next. */
         set_global_state( STATE_PARAMS, global );
         set_global_instr( instr_bytecode, global );
      } else {
         set_global_state( STATE_NONE, global );
      }

      write_double_bin_instr_or_data( (unsigned char)instr_bytecode, global );

      if( STATE_NONE == global->state ) {
         /* Instr has no args, so insert padding to keep IPC divisible by 4. */
         write_double_bin_instr_or_data( 0, global );
      }

      reset_token( global );
      break;

   case STATE_NUM:
      /* Decode token as number. */
      assm_dprintf( 1, "double num: %d", atoi( global->token ) );
      write_double_bin_instr_or_data( atoi( global->token ), global );
      set_global_state( STATE_NONE, global );
      reset_token( global );
      break;

   case STATE_ALLOC:
      /* Allocs are special labels with memory index instead of file offset. */
      add_label( global->token, global->next_alloc_mid, &(global->labels) );
      label_ipc = global->next_alloc_mid;
      global->next_alloc_mid++;

      /* As with label offsets, MIDs are always 16 bits wide. */
      write_double_bin_instr_or_data( label_ipc, global );

      /* Reset state. */
      set_global_state( STATE_NONE, global );
      set_global_instr( 0, global );
      reset_token( global );
      break;

   case STATE_LABEL:
      /* Decode token as label param (or alloc). */
      label_ipc = get_label_ipc( global->token, global->ipc, global );

      /* Label offsets are always 16 bits wide. */
      write_double_bin_instr_or_data( label_ipc, global );

      /* Reset state. */
      set_global_state( STATE_NONE, global );
      set_global_instr( 0, global );
      reset_token( global );
      break;

   case STATE_SYSC:
      assert( 0 < strlen( global->token ) );

      instr_bytecode = token_to_sysc( global->token );
      assm_dprintf( 1, "param: %s%s", global->token,
         0 < instr_bytecode ? " (%d)" : "(invalid token)" );

      assert( 0 < instr_bytecode );

      write_double_bin_instr_or_data( (unsigned char)instr_bytecode, global );

      set_global_state( STATE_NONE, global );
      set_global_instr( 0, global );
      reset_token( global );
      break;

   }
}

void set_global_state( int state, struct ASSM_STATE* global ) {
   assm_dprintf( 2, "state: %s", gc_assm_states[state] );
   global->state = state;
}

void process_char( char c, struct ASSM_STATE* global ) {
   int instr_bytecode = 0,
      instr_args = 0;

   switch( c ) {
   case '.':
      if( STATE_NONE == global->state ) {
         set_global_state( STATE_SECTION, global );
         reset_token( global );

      } else if(
         STATE_STRING == global->state || STATE_CHAR == global->state
      ) {
         append_to_string( c, global );
      }
      break;

   case ':':
      if( STATE_SECTION == global->state ) {
         assm_dprintf( 1, "section: %s", global->token );
         if( 0 == strncmp( "cpu", global->token, 3 ) ) {
            write_bin_instr_or_data( VM_OP_SECT, global );
            write_bin_instr_or_data( VM_SECTION_CPU, global );
            set_global_section( VM_SECTION_CPU, global );

         } else if( 0 == strncmp( "data", global->token, 4 ) ) {
            write_bin_instr_or_data( VM_OP_SECT, global );
            write_bin_instr_or_data( VM_SECTION_DATA, global );
            set_global_section( VM_SECTION_DATA, global );

         } else {
            assm_eprintf( "invalid section: %s", global->token );
            assert( 1 == 0 );
         }
         set_global_state( STATE_NONE, global );
         reset_token( global );

      } else if( STATE_NONE == global->state ) {
         add_label( global->token, global->ipc, &(global->labels) );
         /* TODO: Set state? */
         reset_token( global );

      } else if(
         STATE_STRING == global->state || STATE_CHAR == global->state
      ) {
         append_to_string( c, global );
      }
      break;

   case ';':
      if(
         STATE_NONE == global->state ||
         STATE_PARAMS == global->state ||
         STATE_NUM == global->state
      ) {
         set_global_state( STATE_COMMENT, global );

      } else if(
         STATE_STRING == global->state || STATE_CHAR == global->state
      ) {
         append_to_string( c, global );
      }
      break;

   case '#':
      if( STATE_STRING == global->state || STATE_CHAR == global->state ) {
         append_to_string( c, global );

      } else if( STATE_PARAMS == global->state ) {
         set_global_state( STATE_NUM, global );
      }
      break;

   case '$':
      if( STATE_STRING == global->state || STATE_CHAR == global->state ) {
         append_to_string( c, global );

      } else if( STATE_PARAMS == global->state ) {
         append_to_token( c, global );
         set_global_state( STATE_ALLOC, global );
      }
      break;

   case '"':
      /* Start or terminate string. */
      /* TODO: Handled escaped quote. */
      if( STATE_STRING == global->state ) {
         write_bin_instr_or_data( 0, global ); /* Append NULL byte. */
         set_global_state( STATE_NONE, global );

      } else {
         assert( STATE_NONE == global->state );
         set_global_state( STATE_STRING, global );
      }
      break;

   case '\'':
      if( VM_SECTION_CPU == global->section ) {
         if( STATE_PARAMS == global->state ) {
            set_global_state( STATE_CHAR, global );

         } else if( STATE_CHAR == global->state ) {
            set_global_state( STATE_NONE, global );
            reset_token( global );

         } else if( STATE_COMMENT == global->state ) {
            /* Do nothing. */

         } else {
            append_to_string( c, global );

         }
      }
      break;

   case '\\':
      if(
         (STATE_CHAR == global->state || STATE_STRING == global->state) &&
         !(ASSM_FLAG_ESCAPE == global->flags & ASSM_FLAG_ESCAPE)
      ) {
         global_set_flags( ASSM_FLAG_ESCAPE, global );

      } else if(
         (STATE_CHAR == global->state || STATE_STRING == global->state) &&
         (ASSM_FLAG_ESCAPE == global->flags & ASSM_FLAG_ESCAPE)
      ) {
         global_unset_flags( ASSM_FLAG_ESCAPE, global );
         append_to_string( c, global );

      }
      break;

   case '+':
      if( STATE_STRING == global->state || STATE_CHAR == global->state ) {
         append_to_string( c, global );

      } else if( STATE_PARAMS == global->state ) {
         set_global_state( STATE_NUM, global );
      }
      break;

   case '\n':
   case '\r':
   case ' ':
      if( STATE_STRING == global->state || STATE_CHAR == global->state ) {
         append_to_string( c, global );

      } else if( STATE_COMMENT == global->state && '\n' == c || '\r' == c ) {
         /* Comments end at newline. */
         set_global_state( STATE_NONE, global );

      } else {
         process_token( global );
      }
      break;

   default:
      if( STATE_CHAR == global->state || STATE_STRING == global->state ) {
         if(
            (ASSM_FLAG_ESCAPE == global->flags & ASSM_FLAG_ESCAPE) &&
            'r' == c
         ) {
            c = '\r';
            global_unset_flags( ASSM_FLAG_ESCAPE, global );

         } else if(
            (ASSM_FLAG_ESCAPE == global->flags & ASSM_FLAG_ESCAPE) &&
            'n' == c
         ) {
            c = '\n';
            global_unset_flags( ASSM_FLAG_ESCAPE, global );

         } else if(
            (ASSM_FLAG_ESCAPE == global->flags & ASSM_FLAG_ESCAPE) &&
            't' == c
         ) {
            c = '\t';
            global_unset_flags( ASSM_FLAG_ESCAPE, global );

         } else {
            assert( !(ASSM_FLAG_ESCAPE == (global->flags & ASSM_FLAG_ESCAPE)) );

         }
         append_to_string( c, global );

      } else if( STATE_COMMENT == global->state ) {
         /* Do nothing. */

      } else if( STATE_PARAMS == global->state ) {
         if( VM_OP_SYSC == global->instr & ASSM_MASK_OP ) {
            /* Last instruction was syscall, so expect a syscall. */
            set_global_state( STATE_SYSC, global );
         } else {
            /* Expect a label to jump to/push/etc. */
            set_global_state( STATE_LABEL, global );
         }
         append_to_token( c, global );

      } else if(
         STATE_NONE == global->state &&
         0 == strlen( global->token ) &&
         '0' <= c && '9' >= c
      ) {
         assm_eprintf( "cannot start token with number" );
         assert( 1 == 0 );

      } else {
         append_to_token( c, global );
      }
      break;
   }
}

int assm_resolve_labels( struct ASSM_STATE* global ) {
   struct ASSM_LABEL* unresolved_iter = NULL;
   unsigned short resolve_ipc = 0;
   int resolved_count = 0;

   unresolved_iter = global->unresolved;
   assm_dprintf( 2, "resolving labels..." );
   while( NULL != unresolved_iter ) {
      assm_dprintf( 1, "unres: %s", unresolved_iter->name );

      resolve_ipc = 
         get_label_ipc( unresolved_iter->name, unresolved_iter->ipc, global );

      /* TODO: Fail if resolve failed. */

      /* fseek( bin_file, unresolved_iter->ipc, SEEK_SET ); */
      global->out_buffer_pos = unresolved_iter->ipc;
      global->ipc = unresolved_iter->ipc;
      write_double_bin_instr_or_data( resolve_ipc, global );

      resolved_count++;

      assm_dprintf( 1, "resolved to %d, placed at %d",
         resolve_ipc, unresolved_iter->ipc );

      unresolved_iter = unresolved_iter->next;
   }

   return resolved_count;
}

#ifdef ASSM_MAIN

int main( int argc, char* argv[] ) {
   int bytes_read = 0,
      i = 0;
   char buf[BUF_SZ + 1] = { 0 };
   struct ASSM_STATE global;
   FILE* src_file = NULL,
      * bin_file = NULL;

   /* Initialize state struct. */
   memset( &global, '\0', sizeof( struct ASSM_STATE ) );
   global.next_alloc_mid = 1;

   if( 2 >= argc ) {
      printf( "elix cross-assembler\n\n" );
      printf( "usage: %s <input source file> <binary output>\n\n", argv[0] );
   }

   src_file = fopen( argv[1], "r" );
   if( NULL == src_file ) {
      goto cleanup;
   }

   bytes_read = fread( &buf, sizeof( char ), BUF_SZ, src_file );
   while( 0 < bytes_read ) {
      assm_dprintf( 0, "read: %s", buf );
      for( i = 0 ; bytes_read > i ; i++ ) {
         process_char( buf[i], &global );
      }
      bytes_read = fread( &buf, sizeof( char ), BUF_SZ, src_file );
   }
   memset( buf, '\0', BUF_SZ );

   assm_resolve_labels( &global );
   
   bin_file = fopen( argv[2], "wb" );
   if( NULL == bin_file ) {
      goto cleanup;
   }
   fwrite( global.out_buffer, sizeof( char ), global.out_buffer_len, bin_file );

cleanup:

   if( NULL != src_file ) {
      fclose( src_file );
   }
   if( NULL != bin_file ) {
      fclose( bin_file );
   }

   return 0;
}

#endif /* ASSM_MAIN */

