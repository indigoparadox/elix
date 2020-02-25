
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct replex {
   char c;
   struct replex* peer_up;
   struct replex* peer_down;
   struct replex* next;
};

struct replex* add_char( struct replex* branch, char c ) {
   if( '\0' == branch->c ) {
      printf( "placed %c\n", c );
      branch->c = c;
      return branch;
   } else if( c == branch->c ) {
      printf( "found %c\n", c );
      return branch;
   } else if( c > branch->c ) {
      printf( "diving into peer after %c\n", branch->c );
      if( NULL == branch->peer_up ) {
         branch->peer_up = calloc( 1, sizeof( struct replex ) );
      }
      return add_char( branch->peer_up, c );
   } else {
      printf( "diving into peer before %c\n", branch->c );
      if( NULL == branch->peer_down ) {
         branch->peer_down = calloc( 1, sizeof( struct replex ) );
      }
      return add_char( branch->peer_down, c );
   }
}

int main( void ) {
   FILE* f = NULL;
   char buffer[4096];
   int pos = 0;
   struct replex root = { 'm', NULL, NULL, NULL };
   struct replex* last_char = &root;

   f = fopen( "repl.lex", "r" );

   while( NULL != fgets( buffer, 4096, f ) ) {

      printf( "\nline: %s", buffer );
 
      for( pos = 0 ; strlen( buffer ) > pos ; pos++ ) {
         if( '\n' == buffer[pos] ) {
            continue;
         }

         last_char = add_char( last_char, buffer[pos] );

         printf( "--- next char (under %c) ---\n", last_char->c );

         if( NULL == last_char->next ) {
            last_char->next = calloc( 1, sizeof( struct replex ) );
            last_char->next->c = 'm';
         }

         last_char = last_char->next;

         #if 0
         while( 1 ) {
            if( '\0' == iter->c ) {
               /* This is an empty node. */
               if( diver == depth ) {
                  /* Place this char in this node. */
                  iter->c = buffer[peer];
                  printf( "l%d: placed %c under %c\n",
                     depth, iter->c, prev->c );
               } else {
                  if( NULL == iter->child_up ) {
                     iter->child_up = calloc( 1, sizeof( struct replex ) );
                  }
                  diver++;
               }
            
            } else if( buffer[peer] > iter->c ) {
               if( NULL == iter->peer_up ) {
                  iter->peer_up = calloc( 1, sizeof( struct replex ) );
               }

               prev = iter;
               iter = iter->peer_up;
               printf( "l%d: placing %c under %c->upper\n",
                  depth, buffer[peer], prev->c );
               continue;

            } else {
               if( NULL == iter->peer_down ) {
                  iter->peer_down = calloc( 1, sizeof( struct replex ) );
               }
               prev = iter;
               iter = iter->peer_down;
               printf( "l%d: placing %c under %c->down\n",
                  depth, buffer[peer], prev->c );
               continue;
            }

            break;
         }
         #endif
      }
   }

   fclose( f );

   return 0;
}

