
#include "assm.h"
#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUF_SZ 4096

#define STATE_NONE      0
#define STATE_SECTION   1
#define STATE_PARAMS    2
#define STATE_CHAR      3
#define STATE_STRING    4
#define STATE_NUM       5
#define STATE_COMMENT   6

struct label {
   size_t ipc;
   char name[BUF_SZ + 1];
   struct label* next;
};

struct label* g_labels = NULL;
size_t g_ipc = 0;
int g_section = 0;
int g_escape = 0;
int g_instr = 0;
int g_state = STATE_NONE;
char g_token[BUF_SZ + 1] = { 0 };
size_t g_token_len = 0;
FILE* g_src_file = NULL,
   * g_bin_file = NULL;
uint8_t g_next_alloc_mid = 0;

void reset_token() {
   memset( g_token, '\0', BUF_SZ + 1 );
   g_token_len = 0;
}

void add_label( char* name, int ipc ) {
   struct label* label_iter = g_labels;

   if( NULL == g_labels ) {
      g_labels = calloc( 1, sizeof( struct label ) );
      assert( NULL != g_labels );
      label_iter = g_labels;

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

   printf( "label: %s @ %d\n", name, ipc );
}

size_t get_label_ipc( char* label ) {
   struct label* label_iter = g_labels;

   while(
      NULL != label_iter &&
      0 != strncmp( label_iter->name, label, strlen( label_iter->name ) )
   ) {
      printf( "searching for %s, found %s...\n", label, label_iter->name );
      label_iter = label_iter->next;
   }

   if( NULL == label_iter ) {
      printf( "label not found\n" );
      return NULL;
   }

   printf( "label for %s: %d\n", label, (unsigned char)(label_iter->ipc) );

   return label_iter->ipc;
}

void write_bin_instr_or_data( char c ) {
   fwrite( &c, sizeof( char ), 1, g_bin_file );
   g_ipc++;
   printf( "wrote %d (0x%x), ipc: %ld\n", c, c, g_ipc );
}

void append_to_token( char c ) {
   assert( g_token_len < BUF_SZ );
   g_token[g_token_len] = c;
   g_token_len++;
}

void process_char( char c ) {
   char buf_out[BUF_SZ + 1] = { 0 };
   size_t label_ipc = 0;
   int instr_bytecode = 0;

   switch( c ) {
   case '.':
      if( STATE_NONE == g_state ) {
         printf( "state section\n" );
         g_state = STATE_SECTION;
         reset_token();
      } else if( STATE_STRING == g_state ) {
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );
      }
      break;

   case ':':
      if( STATE_SECTION == g_state ) {
         printf( "section: %s\n", g_token );
         if( 0 == strncmp( "cpu", g_token, 3 ) ) {
            write_bin_instr_or_data( VM_INSTR_SECT );
            write_bin_instr_or_data( VM_SECTION_CPU );
            g_section = VM_SECTION_CPU;

         } else if( 0 == strncmp( "data", g_token, 4 ) ) {
            write_bin_instr_or_data( VM_INSTR_SECT );
            write_bin_instr_or_data( VM_SECTION_DATA );
            g_section = VM_SECTION_DATA;

         }
         reset_token();
         g_state = STATE_NONE;

      } else if( STATE_NONE == g_state ) {
         add_label( g_token, g_ipc );
         reset_token();

      } else if( STATE_STRING == g_state ) {
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );
      }
      break;

   case ';':
      if( STATE_NONE == g_state ) {
         g_state = STATE_COMMENT;
      } else if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );
      }
      break;

   case '#':
      if( STATE_STRING == g_state ) {
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );
      } else if( STATE_PARAMS == g_state ) {
         g_state = STATE_NUM;
      }
      break;

   case '"':
      /* Start or terminate string. */
      if( STATE_STRING == g_state ) {
         write_bin_instr_or_data( 0 ); /* Append NULL byte. */
         printf( "state none\n" );
         g_state = STATE_NONE;

      } else {
         assert( STATE_NONE == g_state );
         printf( "state string\n" );
         g_state = STATE_STRING;
      }
      break;

   case '\'':
      if( VM_SECTION_CPU == g_section ) {
         if( STATE_PARAMS == g_state ) {
            printf( "state params\n" );
            g_state = STATE_CHAR;

         } else if( STATE_CHAR == g_state ) {
            printf( "state none\n" );
            reset_token();
            g_state = STATE_NONE;

         } else {
            printf( "c: %c\n", c );
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
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );

      }
      break;

   case '\n':
   case '\r':
   case ' ':
      if( STATE_NONE == g_state ) {
         /* Try to decode token as an instruction. */
         instr_bytecode = -1;
         if( 0 == strncmp( "push", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_PUSH;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_PUSH;

         } else if( 0 == strncmp( "syscall", g_token, 7 ) ) {
            instr_bytecode = VM_INSTR_SYSC;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_SYSC;

         } else if( 0 == strncmp( "goto", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_GOTO;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_GOTO;
         
         } else if( 0 == strncmp( "pop", g_token, 3 ) ) {
            instr_bytecode = VM_INSTR_POP;
            g_state = STATE_NONE;

         } else if( 0 == strncmp( "jsnz", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_JSNZ;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_JSNZ;
         
         } else if( 0 == strncmp( "jseq", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_JSEQ;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_JSEQ;
         
         } else if( 0 == strncmp( "jsne", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_JSNE;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_JSNE;
         
         }
         
         if( 0 <= instr_bytecode ) {
            printf( "instr: %s ", g_token );
            printf( "(%d)\n", instr_bytecode );
            write_bin_instr_or_data( (unsigned char)instr_bytecode );
         } else {
            //printf( "(bad instruction)\n" );
         }
         reset_token();

      } else if( STATE_COMMENT == g_state && ('\n' == c || '\r' == c) ) {
         g_state = STATE_NONE;

      } else if( STATE_STRING == g_state ) {
         /* Add whitespace character to string literal. */
         write_bin_instr_or_data( c );

      } else if( STATE_NUM == g_state ) {
         /* Decode token as number. */
         printf( "num: %d\n", atoi( g_token ) );
         write_bin_instr_or_data( atoi( g_token ) );
         reset_token();
         g_state = STATE_NONE;

      } else if(
         (VM_INSTR_GOTO == g_instr ||
            VM_INSTR_PUSH == g_instr ||
            VM_INSTR_JSNZ == g_instr ||
            VM_INSTR_JSNE == g_instr ||
            VM_INSTR_JSEQ == g_instr) &&
         STATE_PARAMS == g_state &&
         0 < g_token_len
      ) {
         /* Decode token as label (or alloc). */
         label_ipc = get_label_ipc( g_token );
         if( '$' == g_token[0] ) {
            /* Token is an alloc, so create if doesn't exist. */
            add_label( g_token, g_next_alloc_mid );
            label_ipc = g_next_alloc_mid;
            g_next_alloc_mid++;
         }
         write_bin_instr_or_data( (unsigned char)label_ipc );
         g_state = STATE_NONE;
         g_instr = 0;
         reset_token();

      } else if( STATE_PARAMS == g_state ) {
         if( 0 < g_token_len ) {
            printf( "param: %s", g_token );
            instr_bytecode = -1;

            if( 0 == strncmp( g_token, "putc", 4 ) ) {
               instr_bytecode = VM_SYSC_PUTC;

            } else if( 0 == strncmp( g_token, "printf", 6 ) ) {
               instr_bytecode = VM_SYSC_PRINTF;

            } else if( 0 == strncmp( g_token, "alloc", 5 ) ) {
               instr_bytecode = VM_SYSC_ALLOC;

            } else if( 0 == strncmp( g_token, "mstore", 6 ) ) {
               instr_bytecode = VM_SYSC_MSTORE;

            } else if( 0 == strncmp( g_token, "mget", 4 ) ) {
               instr_bytecode = VM_SYSC_MGET;

            } else if( 0 == strncmp( g_token, "getc", 4 ) ) {
               instr_bytecode = VM_SYSC_GETC;

            }

            if( 0 <= instr_bytecode ) {
               printf( " (%d)\n", instr_bytecode );
               write_bin_instr_or_data( (unsigned char)instr_bytecode );
            } else {
               printf( "\n" );
            }
  
            g_state = STATE_NONE;
            g_instr = 0;
            reset_token();
         }
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

      } else {
         append_to_token( c );
      }
      break;
   }
   
   //printf( "c: %c\n", c );
}

int main( int argc, char* argv[] ) {
   int bytes_read = 0,
      i = 0;
   char buf[BUF_SZ + 1] = { 0 };

   assert( 2 < argc );

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
      printf( "%s\n", buf );

      for( i = 0 ; bytes_read > i ; i++ ) {
         process_char( buf[i] );
      }

      bytes_read = fread( &buf, sizeof( char ), BUF_SZ, g_src_file );
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
