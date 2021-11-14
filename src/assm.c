
#define ASSM_C
#define VM_ASSM
#include "assm.h"
#include "vm.h"
#include "sysc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SZ 40960

struct label {
   unsigned short ipc;
   char name[BUF_SZ + 1];
   struct label* next;
};

struct label* g_labels = NULL;
struct label* g_unresolved = NULL;
unsigned short g_ipc = 0;
int g_section = 0;
int g_escape = 0;
int g_instr = 0;
int g_state = STATE_NONE;
char g_token[BUF_SZ + 1] = { 0 };
unsigned short g_token_len = 0;
FILE* g_src_file = NULL,
   * g_bin_file = NULL;
uint8_t g_next_alloc_mid = 1;

void reset_token() {
   memset( g_token, '\0', BUF_SZ + 1 );
   g_token_len = 0;
}

void add_label( char* name, int ipc, struct label** head ) {
   struct label* label_iter = *head;

   if( NULL == *head ) {
      *head = calloc( 1, sizeof( struct label ) );
      assert( NULL != *head );
      label_iter = *head;

   } else {
      while( NULL != label_iter->next ) {
         label_iter = label_iter->next;
      }
      label_iter->next = calloc( 1, sizeof( struct label ) );
      assert( NULL != label_iter->next );
      label_iter = label_iter->next;
   }

   strncpy( label_iter->name, name, BUF_SZ );
   label_iter->ipc = ipc;

   assm_dprintf( 1, "label: %s @ %d%s", name, ipc,
      '$' == name[0] ? " (dynamic)" : "" );
}

unsigned short get_label_ipc( char* label, unsigned short ipc_of_call ) {
   struct label* label_iter = g_labels;

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
      assm_dprintf( 0, "searching for %s, found %s (%d)...",
         label, label_iter->name, label_iter->ipc );
      assert( 0 == strncmp(
         label_iter->name, label, strlen( label_iter->name ) ) );
      assert( strlen( label ) == strlen( label_iter->name ) );
   }

   if( NULL == label_iter ) {
      if( '$' != label[0] ) {
         assm_eprintf( "label not found; added to unresolved list" );
         add_label( label, ipc_of_call, &g_unresolved );
      }
      return 0;
   }

   assm_dprintf( 1, "label for %s: %d", label, label_iter->ipc );

   return label_iter->ipc;
}

void write_bin_instr_or_data( unsigned char c ) {
   fwrite( &c, sizeof( char ), 1, g_bin_file );
   g_ipc++;
   assm_dprintf( 1, "wrote %d (0x%x), ipc: %d", c, c, g_ipc );
}

void write_double_bin_instr_or_data( unsigned short u ) {
   assm_dprintf( 1, "first byte:" );
   write_bin_instr_or_data( (uint8_t)((u >> 8) & 0x00ff) );
   assm_dprintf( 1, "second byte:" );
   write_bin_instr_or_data( (uint8_t)(u & 0x00ff) );
   assm_dprintf( 1, "previous writes wrote %d (0x%x) (0x%x 0x%x), ipc: %d, %d",
      u,
      u,
      (uint8_t)((u >> 8) & 0x00ff),
      (uint8_t)(u & 0x00ff),
      g_ipc - 1,
      g_ipc);
}

void append_to_token( char c ) {
   assert( g_token_len < BUF_SZ );
   g_token[g_token_len] = c;
   g_token_len++;
}

void append_to_string( char c ) {
   assm_dprintf( 1, "str c: %c", c );
   write_bin_instr_or_data( c );
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
      }
      goto cleanup;
   }

   if( 'd' == token[strlen( gc_vm_op_tokens[op_out] )] ) {
      /* Set the high bit to indicate op on a double. */
      op_out |= 0x80;
   }

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
   }

   return sysc_out;
}

void set_global_instr( int instr_bytecode ) {
   if( 0 == instr_bytecode ) {
      assm_dprintf( 1, "no longer processing instruction" );
   } else {
      assm_dprintf( 1, "processing instruction: %s",
         gc_vm_op_tokens[instr_bytecode & 0x7f] );
   }
   g_instr = instr_bytecode;
}

void process_token( char* token ) {
   int instr_bytecode = 0;
   unsigned short label_ipc = 0;

   switch( g_state ) {
   case STATE_NONE:
      /* Try to decode token as an instruction. */
      instr_bytecode = token_to_op( g_token );
      if( 0 < instr_bytecode ) {
         assm_dprintf( 2, "token: %s (%d)",
            gc_vm_op_tokens[instr_bytecode & 0x7f],
            instr_bytecode );
         if( 0 < gc_vm_op_argcs[instr_bytecode & 0x7f] ) {
            set_global_state( STATE_PARAMS );
            set_global_instr( instr_bytecode );
         } else {
            set_global_state( STATE_NONE );
         }

         write_bin_instr_or_data( (unsigned char)instr_bytecode );
         if(
            0 == gc_vm_op_argcs[instr_bytecode & 0x7f] ||
            0x80 == instr_bytecode & 0x80
         ) {
            /* Stack instruction has no data or 16-bit-wide data,
               so put a null padding in instruction data field. */
            assm_dprintf( 1, "padding:" );
            write_bin_instr_or_data( 0 );
         }

      } else if( 0 < strlen( g_token ) ) {
         assm_eprintf( "unknown instruction: %s", g_token );
         assert( 1 == 0 );
      }

      reset_token();
      break;

   case STATE_NUM:
      /* Decode token as number. */
      if(
         0x80 == g_instr & 0x80 &&
         0 < gc_vm_op_argcs[g_instr & 0x7f]
      ) {
         write_double_bin_instr_or_data( atoi( g_token ) );
      } else {
         assm_dprintf( 1, "num: %d", atoi( g_token ) );
         write_bin_instr_or_data( atoi( g_token ) );
      }
      set_global_state( STATE_NONE );
      reset_token();
      break;

   case STATE_ALLOC:
      add_label( g_token, g_next_alloc_mid, &g_labels );
      label_ipc = g_next_alloc_mid;
      g_next_alloc_mid++;
      set_global_state( STATE_NONE );
      set_global_instr( 0 );
      reset_token();
      break;

   case STATE_LABEL:
      /* Decode token as label param (or alloc). */
      label_ipc = get_label_ipc( g_token, g_ipc );
      if(
         1 == gc_vm_op_argcs[instr_bytecode & 0x7f] &&
         0x80 == instr_bytecode & 0x80
      ) {
         write_double_bin_instr_or_data( label_ipc );
      } else {
         write_bin_instr_or_data( (unsigned char)label_ipc );
      }
      set_global_state( STATE_NONE );
      set_global_instr( 0 );
      reset_token();
      break;

   case STATE_SYSC:
      if( 0 < g_token_len ) {
         instr_bytecode = token_to_sysc( g_token );
         assm_dprintf( 1, "param: %s%s", g_token,
            0 < instr_bytecode ? " (%d)" : "(invalid token)" );

         if( 0 < instr_bytecode ) {
            write_bin_instr_or_data( (unsigned char)instr_bytecode );
         }

         set_global_state( STATE_NONE );
         set_global_instr( 0 );
         reset_token();
      }
      break;

   }
}

void set_global_state( int state ) {
   assm_dprintf( 2, "state: %s", gc_assm_states[state] );
   g_state = state;
}

void process_char( char c ) {
   char buf_out[BUF_SZ + 1] = { 0 };
   int instr_bytecode = 0,
      instr_args = 0;

   switch( c ) {
   case '.':
      if( STATE_NONE == g_state ) {
         assm_dprintf( 1, "state section" );
         set_global_state( STATE_SECTION );
         reset_token();
      } else if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );
      }
      break;

   case ':':
      if( STATE_SECTION == g_state ) {
         assm_dprintf( 1, "section: %s", g_token );
         if( 0 == strncmp( "cpu", g_token, 3 ) ) {
            write_bin_instr_or_data( VM_OP_SECT );
            write_bin_instr_or_data( VM_SECTION_CPU );
            g_section = VM_SECTION_CPU;

         } else if( 0 == strncmp( "data", g_token, 4 ) ) {
            write_bin_instr_or_data( VM_OP_SECT );
            write_bin_instr_or_data( VM_SECTION_DATA );
            g_section = VM_SECTION_DATA;

         } else {
            assm_eprintf( "invalid section: %s", g_token );
            assert( 1 == 0 );
         }
         set_global_state( STATE_NONE );
         reset_token();

      } else if( STATE_NONE == g_state ) {
         add_label( g_token, g_ipc, &g_labels );
         /* TODO: Set state? */
         reset_token();

      } else if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );
      }
      break;

   case ';':
      if(
         STATE_NONE == g_state ||
         STATE_PARAMS == g_state ||
         STATE_NUM == g_state
      ) {
         set_global_state( STATE_COMMENT );
      } else if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );
      }
      break;

   case '#':
      if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );
      } else if( STATE_PARAMS == g_state ) {
         set_global_state( STATE_NUM );
      }
      break;

   case '$':
      if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );
      } else if( STATE_PARAMS == g_state ) {
         append_to_token( c );
         set_global_state( STATE_ALLOC );
      }
      break;

   case '"':
      /* Start or terminate string. */
      /* TODO: Handled escaped quote. */
      if( STATE_STRING == g_state ) {
         write_bin_instr_or_data( 0 ); /* Append NULL byte. */
         set_global_state( STATE_NONE );

      } else {
         assert( STATE_NONE == g_state );
         set_global_state( STATE_STRING );
      }
      break;

   case '\'':
      if( VM_SECTION_CPU == g_section ) {
         if( STATE_PARAMS == g_state ) {
            set_global_state( STATE_CHAR );

         } else if( STATE_CHAR == g_state ) {
            set_global_state( STATE_NONE );
            reset_token();

         } else if( STATE_COMMENT == g_state ) {
            /* Do nothing. */

         } else {
            assm_dprintf( 1, "c: %c", c );
            write_bin_instr_or_data( c );

         }
      }
      break;

   case '\\':
      if( (STATE_CHAR == g_state || STATE_STRING == g_state) && !g_escape ) {
         g_escape = 1;

      } else if(
         (STATE_CHAR == g_state || STATE_STRING == g_state) && g_escape
      ) {
         g_escape = 0;
         append_to_string( c );

      }
      break;

   case '+':
      if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );

      } else if( STATE_PARAMS == g_state ) {
         set_global_state( STATE_NUM );
      }
      break;

   case '\n':
   case '\r':
   case ' ':
      if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         append_to_string( c );

      } else if( STATE_COMMENT == g_state && '\n' == c || '\r' == c ) {
         /* Comments end at newline. */
         set_global_state( STATE_NONE );

      } else {
         process_token( g_token );
      }
      break;

   default:
      if( STATE_CHAR == g_state || STATE_STRING == g_state ) {
         if( g_escape && 'r' == c ) {
            c = '\r';
            g_escape = 0;
         } else if( g_escape && 'n' == c ) {
            c = '\n';
            g_escape = 0;
         } else if( g_escape && 't' == c ) {
            c = '\t';
            g_escape = 0;
         } else {
            assert( !g_escape );
         }
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );

      } else if( STATE_COMMENT == g_state ) {
         /* Do nothing. */

      } else if( STATE_PARAMS == g_state ) {
         if( VM_OP_SYSC == g_instr & 0x7f ) {
            /* Last instruction was syscall, so expect a syscall. */
            set_global_state( STATE_SYSC );
         } else {
            /* Expect a label to jump to/push/etc. */
            set_global_state( STATE_LABEL );
         }
         append_to_token( c );

      } else {
         append_to_token( c );
      }
      break;
   }
   
   //printf( "c: %c\n", c );
}

#ifdef ASSM_MAIN

int main( int argc, char* argv[] ) {
   int bytes_read = 0,
      i = 0;
   char buf[BUF_SZ + 1] = { 0 };
   struct label* unresolved_iter = NULL;
   unsigned short resolve_ipc = 0;

   if( 2 >= argc ) {
      printf( "elix cross-assembler\n\n" );
      printf( "usage: %s <input source file> <binary output>\n\n", argv[0] );
   }

   g_src_file = fopen( argv[1], "r" );
   if( NULL == g_src_file ) {
      goto cleanup;
   }

   g_bin_file = fopen( argv[2], "wb" );
   if( NULL == g_bin_file ) {
      goto cleanup;
   }

   bytes_read = fread( &buf, sizeof( char ), BUF_SZ, g_src_file );
   while( 0 < bytes_read ) {
      assm_dprintf( 0, "read: %s", buf );
      for( i = 0 ; bytes_read > i ; i++ ) {
         process_char( buf[i] );
      }
      bytes_read = fread( &buf, sizeof( char ), BUF_SZ, g_src_file );
   }
   memset( buf, '\0', BUF_SZ );

   unresolved_iter = g_unresolved;
   while( NULL != unresolved_iter ) {
      assm_dprintf( 1, "unres: %s", unresolved_iter->name );

      resolve_ipc = 
         get_label_ipc( unresolved_iter->name, unresolved_iter->ipc );

      fseek( g_bin_file, unresolved_iter->ipc, SEEK_SET );
      g_ipc = unresolved_iter->ipc;
      write_double_bin_instr_or_data( resolve_ipc );

      assm_dprintf( 1, "resolved to %d, placed at %d",
         resolve_ipc, unresolved_iter->ipc );

      unresolved_iter = unresolved_iter->next;
   }

cleanup:

   if( NULL != g_src_file ) {
      fclose( g_src_file );
   }
   if( NULL != g_bin_file ) {
      fclose( g_bin_file );
   }

   return 0;
}

#endif /* ASSM_MAIN */

