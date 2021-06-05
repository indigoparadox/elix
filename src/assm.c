
#include "assm.h"
#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUF_SZ 255

#define STATE_NONE      0
#define STATE_SECTION   1
#define STATE_PARAMS    2
#define STATE_CHAR      3

struct label {
   size_t ipc;
   char name[BUF_SZ + 1];
   struct label* next;
};

struct label* g_labels = NULL;
size_t g_ipc = 0;
int g_section = 0;
int g_instr = 0;
int g_state = STATE_NONE;
char g_token[BUF_SZ + 1] = { 0 };
size_t g_token_len = 0;
FILE* g_src_file = NULL,
   * g_bin_file = NULL;

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

   while( 0 != strncmp( label_iter->name, label, strlen( label_iter->name ) ) ) {
      printf( "searching for %s, found %s...\n", label, label_iter->name );
      label_iter = label_iter->next;
   }

   assert( NULL != label_iter );

   printf( "label for %s: %c\n", label, (unsigned char)(label_iter->ipc) );

   return label_iter->ipc;
}

void write_bin_instr_or_data( char c ) {
   fwrite( &c, sizeof( char ), 1, g_bin_file );
   g_ipc++;
   printf( "ipc: %ld\n", g_ipc );
}

void process_char( char c ) {
   char buf_out[BUF_SZ + 1] = { 0 };
   size_t label_ipc = 0;

   switch( c ) {
   case '.':
      if( STATE_NONE == g_state ) {
         printf( "state section\n" );
         g_state = STATE_SECTION;
         reset_token();
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
      } else if( STATE_NONE == g_state && VM_SECTION_CPU == g_section ) {
         add_label( g_token, g_ipc );
         reset_token();
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
         }
      }
      break;

   case '\n':
   case '\r':
   case ' ':
      if( VM_SECTION_CPU == g_section && STATE_NONE == g_state ) {
         //if( STATE_PARAMS == g_state ) {
         //   g_state = STATE_NONE;
         if( 0 == strncmp( "push", g_token, 4 ) ) {
            write_bin_instr_or_data( VM_INSTR_PUSH );
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_PUSH;
         } else if( 0 == strncmp( "syscall", g_token, 7 ) ) {
            write_bin_instr_or_data( VM_INSTR_SYSC );
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_SYSC;
         } else if( 0 == strncmp( "goto", g_token, 4 ) ) {
            write_bin_instr_or_data( VM_INSTR_GOTO );
            g_state = STATE_PARAMS;
            g_instr = VM_INSTR_GOTO;
         }
         if( 0 < g_token_len ) {
            printf( "instr: %s\n", g_token );
         }
         reset_token();
         break;
      } else if(
         VM_SECTION_CPU == g_section &&
         VM_INSTR_GOTO == g_instr &&
         STATE_PARAMS == g_state &&
         0 < g_token_len
      ) {
         label_ipc = get_label_ipc( g_token );
         write_bin_instr_or_data( (unsigned char)label_ipc );
         g_state = STATE_NONE;
         g_instr = 0;
      } else if( VM_SECTION_CPU == g_section && STATE_PARAMS == g_state ) {
         if( 0 < g_token_len ) {
            printf( "param: %s\n", g_token );
            if( 0 == strncmp( g_token, "putc", 4 ) ) {
               write_bin_instr_or_data( VM_SYSC_PUTC );
               g_state = STATE_NONE;
               g_instr = 0;
            }
            reset_token();
         }
         break;
      }

   default:
      if( STATE_CHAR == g_state ) {
         printf( "c: %c\n", c );
         write_bin_instr_or_data( c );
      } else {
         assert( g_token_len < BUF_SZ );
         g_token[g_token_len] = c;
         g_token_len++;
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

