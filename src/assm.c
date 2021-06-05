
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

int g_section = 0;
int g_state = STATE_NONE;
char g_token[BUF_SZ + 1] = { 0 };
size_t g_token_len = 0;
FILE* g_src_file = NULL,
   * g_bin_file = NULL;

void reset_token() {
   memset( g_token, '\0', BUF_SZ + 1 );
   g_token_len = 0;
}

void process_char( char c ) {
   char buf_out[BUF_SZ + 1] = { 0 };

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
            buf_out[0] = VM_INSTR_SECT;
            buf_out[1] = VM_SECTION_CPU;
            fwrite( &buf_out, sizeof( char ), 2, g_bin_file );
            g_section = VM_SECTION_CPU;
         } else if( 0 == strncmp( "data", g_token, 4 ) ) {
            buf_out[0] = VM_INSTR_SECT;
            buf_out[1] = VM_SECTION_DATA;
            fwrite( &buf_out, sizeof( char ), 2, g_bin_file );
            g_section = VM_SECTION_DATA;
         }
         reset_token();
         g_state = STATE_NONE;
      }
      break;

   case '\'':
      if( VM_SECTION_CPU == g_section ) {
         if( STATE_PARAMS == g_state ) {
            printf( "params state\n" );
            g_state = STATE_CHAR;
         } else if( STATE_CHAR == g_state ) {
            printf( "no state\n" );
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
            buf_out[0] = VM_INSTR_PUSH;
            fwrite( &buf_out, sizeof( char ), 1, g_bin_file );
            g_state = STATE_PARAMS;
         } else if( 0 == strncmp( "syscall", g_token, 7 ) ) {
            buf_out[0] = VM_INSTR_SYSC;
            fwrite( &buf_out, sizeof( char ), 1, g_bin_file );
            g_state = STATE_PARAMS;
         }
         printf( "instr: %s\n", g_token );
         reset_token();
         break;
      } else if( VM_SECTION_CPU == g_section && STATE_PARAMS == g_state ) {
         if( 0 < g_token_len ) {
            printf( "param: %s\n", g_token );
            if( 0 == strncmp( g_token, "putc", 4 ) ) {
               buf_out[0] = VM_SYSC_PUTC;
               fwrite( &buf_out, sizeof( char ), 1, g_bin_file );
               g_state = STATE_NONE;
            }
            reset_token();
         }
         break;
      }

   default:
      if( STATE_CHAR == g_state ) {
         fwrite( &c, sizeof( char ), 1, g_bin_file );
      } else {
         assert( g_token_len < BUF_SZ );
         g_token[g_token_len] = c;
         g_token_len++;
      }
      break;
   }
   
   printf( "c: %c\n", c );
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

