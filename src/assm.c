
#include "assm.h"
#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SZ 4096

#define STATE_NONE      0
#define STATE_SECTION   1
#define STATE_PARAMS    2
#define STATE_CHAR      3
#define STATE_STRING    4
#define STATE_NUM       5
#define STATE_COMMENT   6
#define STATE_PLUS      7

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
int g_plus = 0;
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

   printf( "label: %s @ %d", name, ipc );
   if( '$' == name[0] ) {
      printf( " (dynamic)" );
   }
   printf( "\n" );
}

unsigned short get_label_ipc( char* label, unsigned short ipc_of_call ) {
   struct label* label_iter = g_labels;

   while(
      NULL != label_iter &&
      0 != strncmp( label, label_iter->name, strlen( label ) )
   ) {
      printf( "searching for %s, found %s (%d)...\n",
         label, label_iter->name, label_iter->ipc );
      label_iter = label_iter->next;
   }
   if( NULL != label_iter ) {
      printf( "searching for %s, found %s (%d)...\n",
         label, label_iter->name, label_iter->ipc );
   }

   if( NULL == label_iter ) {
      if( '$' != label[0] ) {
         printf( "label not found; added to unresolved list\n" );
         add_label( label, ipc_of_call, &g_unresolved );
      }
      return 0;
   }

   printf( "label for %s: %d\n", label, label_iter->ipc );

   return label_iter->ipc;
}

void write_bin_instr_or_data( unsigned char c ) {
   fwrite( &c, sizeof( char ), 1, g_bin_file );
   g_ipc++;
   printf( "wrote %d (0x%x), ipc: %d\n\n", c, c, g_ipc );
}

void write_double_bin_instr_or_data( unsigned short u ) {
   printf( "first byte:\n" );
   write_bin_instr_or_data( (uint8_t)((u >> 8) & 0x00ff) );
   printf( "second byte:\n" );
   write_bin_instr_or_data( (uint8_t)(u & 0x00ff) );
   printf( "previous writes wrote %d (0x%x) (0x%x 0x%x), ipc: %d, %d\n\n",
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

void process_char( char c ) {
   char buf_out[BUF_SZ + 1] = { 0 };
   unsigned short label_ipc = 0;
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
         add_label( g_token, g_ipc, &g_labels );
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
      /* TODO: Handled escaped quote. */
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

   case '+':
      if( STATE_STRING == g_state || STATE_CHAR == g_state ) {
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );

      } else if( STATE_PARAMS == g_state ) {
         g_state = STATE_PLUS;
         g_plus = 0;
         printf( "state plus" );
      }
      break;

   case '\n':
   case '\r':
   case ' ':
      if( STATE_NONE == g_state ) {
         /* Try to decode token as an instruction. */
         instr_bytecode = -1;
         if( 0 == strncmp( "pushd", g_token, 5 ) ) {
            instr_bytecode = VM_INSTR_PUSHD;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_PUSHD;

         } else if( 0 == strncmp( "push", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_PUSH;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_PUSH;

         } else if( 0 == strncmp( "syscall", g_token, 7 ) ) {
            instr_bytecode = VM_INSTR_SYSC;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_SYSC;

         } else if( 0 == strncmp( "jump", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_JUMP;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_JUMP;
         
         } else if( 0 == strncmp( "spop", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_SPOP;
            g_state = STATE_NONE;

         } else if( 0 == strncmp( "sdpop", g_token, 5 ) ) {
            instr_bytecode = VM_INSTR_SDPOP;
            g_state = STATE_NONE;

         } else if( 0 == strncmp( "saddd", g_token, 5 ) ) {
            instr_bytecode = VM_INSTR_SADDD;
            g_state = STATE_NONE;

         } else if( 0 == strncmp( "sadd", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_SADD;
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
         
         } else if( 0 == strncmp( "jsged", g_token, 5 ) ) {
            instr_bytecode = VM_INSTR_JSGED;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_JSGED;
         
         } else if( 0 == strncmp( "jsge", g_token, 4 ) ) {
            instr_bytecode = VM_INSTR_JSGE;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_JSGE;
         
         } else if( 0 == strncmp( g_token, "malloc", 6 ) ) {
            instr_bytecode = VM_INSTR_MALLOC;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MALLOC;

         } else if( 0 == strncmp( g_token, "mstore", 6 ) ) {
            instr_bytecode = VM_INSTR_MPOP;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPOP;

         } else if( 0 == strncmp( g_token, "mpushco", 7 ) ) {
            instr_bytecode = VM_INSTR_MPUSHCO;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPUSHCO;

         } else if( 0 == strncmp( g_token, "mpushcd", 7 ) ) {
            instr_bytecode = VM_INSTR_MPUSHCD;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPUSHCD;

         } else if( 0 == strncmp( g_token, "mpushc", 6 ) ) {
            instr_bytecode = VM_INSTR_MPUSHC;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPUSHC;

         } else if( 0 == strncmp( g_token, "mpopco", 6 ) ) {
            instr_bytecode = VM_INSTR_MPOPCO;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPOPCO;

         } else if( 0 == strncmp( g_token, "mpopcd", 6 ) ) {
            instr_bytecode = VM_INSTR_MPOPCD;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPOPCD;

         } else if( 0 == strncmp( g_token, "mpopc", 5 ) ) {
            instr_bytecode = VM_INSTR_MPOPC;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPOPC;

         } else if( 0 == strncmp( g_token, "mpopd", 5 ) ) {
            instr_bytecode = VM_INSTR_MPOPD;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPOPD;

         } else if( 0 == strncmp( g_token, "mpop", 4 ) ) {
            instr_bytecode = VM_INSTR_MPOP;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MPOP;

         } else if( 0 == strncmp( g_token, "mfree", 5 ) ) {
            instr_bytecode = VM_INSTR_MFREE;
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_MFREE;
         }
         
         if( 0 <= instr_bytecode ) {
            printf( "instr: %s ", g_token );
            printf( "(%d)\n", instr_bytecode );
            write_bin_instr_or_data( (unsigned char)instr_bytecode );
            if(
               (VM_INSTR_SMIN <= instr_bytecode &&
               VM_INSTR_SMAX >= instr_bytecode) ||
               (VM_INSTR_MMIN <= instr_bytecode &&
               VM_INSTR_MMAX >= instr_bytecode) ||
               (VM_INSTR_JMIN <= instr_bytecode &&
               VM_INSTR_JMAX >= instr_bytecode) ||
               VM_INSTR_PUSHD == instr_bytecode
            ) {
               /* Stack instruction has no data or 16-bit-wide data,
                  so put a null padding in instruction data field. */
               printf( "padding:\n" );
               write_bin_instr_or_data( 0 );
            }
         } else {
            //printf( "(bad instruction)\n" );
         }
         reset_token();

      } else if( STATE_COMMENT == g_state && ('\n' == c || '\r' == c) ) {
         g_state = STATE_NONE;

      } else if( STATE_STRING == g_state ) {
         /* Add whitespace character to string literal. */
         write_bin_instr_or_data( c );

      } else if( STATE_NUM == g_state || STATE_PLUS == g_state ) {
         /* Decode token as number. */
         if(
            VM_INSTR_PUSHD == g_instr ||
            (VM_INSTR_JMIN <= instr_bytecode &&
            VM_INSTR_JMAX >= instr_bytecode) ||
            (VM_INSTR_MMIN <= instr_bytecode &&
            VM_INSTR_MMAX >= instr_bytecode)
         ) {
            write_double_bin_instr_or_data( atoi( g_token ) );
         } else {
            printf( "num: %d\n", atoi( g_token ) );
            write_bin_instr_or_data( atoi( g_token ) );
         }
         reset_token();
         g_state = STATE_NONE;

      } else if(
         (VM_INSTR_PUSH == g_instr ||
            VM_INSTR_PUSHD == g_instr ||
            (VM_INSTR_JMIN <= g_instr && VM_INSTR_JMAX >= g_instr) ||
            (VM_INSTR_MMIN <= g_instr && VM_INSTR_MMAX >= g_instr)) &&
         STATE_PARAMS == g_state &&
         0 < g_token_len
      ) {
         /* Decode token as label (or alloc). */
         label_ipc = get_label_ipc( g_token, g_ipc );
         if( '$' == g_token[0] && 0 == label_ipc ) {
            /* Token is an alloc, so create if doesn't exist. */
            add_label( g_token, g_next_alloc_mid, &g_labels );
            label_ipc = g_next_alloc_mid;
            g_next_alloc_mid++;
         }
         if(
            VM_INSTR_PUSHD == g_instr ||
            (VM_INSTR_MMIN <= g_instr && VM_INSTR_MMAX >= g_instr) ||
            (VM_INSTR_JMIN <= g_instr && VM_INSTR_JMAX >= g_instr)
         ) {
            write_double_bin_instr_or_data( label_ipc );
         } else {
            write_bin_instr_or_data( (unsigned char)label_ipc );
         }
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

            } else if( 0 == strncmp( g_token, "getc", 4 ) ) {
               instr_bytecode = VM_SYSC_GETC;

            } else if( 0 == strncmp( g_token, "droot", 5 ) ) {
               instr_bytecode = VM_SYSC_DROOT;

            } else if( 0 == strncmp( g_token, "dfirst", 6 ) ) {
               instr_bytecode = VM_SYSC_DFIRST;

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
   struct label* unresolved_iter = NULL;
   unsigned short resolve_ipc = 0;

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
   memset( buf, '\0', BUF_SZ );

   unresolved_iter = g_unresolved;
   while( NULL != unresolved_iter ) {
      printf( "unres: %s\n", unresolved_iter->name );

      resolve_ipc = 
         get_label_ipc( unresolved_iter->name, unresolved_iter->ipc );

      fseek( g_bin_file, unresolved_iter->ipc, SEEK_SET );
      g_ipc = unresolved_iter->ipc;
      write_double_bin_instr_or_data( resolve_ipc );

      printf( "resolved to %d, placed at %d\n",
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

