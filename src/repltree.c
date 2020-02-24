
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct replex {
   char c;
   struct replex* peer_up;
   struct replex* peer_down;
   struct replex* child_up;
   struct replex* child_down;
};

int main( void ) {
   FILE* f = NULL;
   char buffer[4096];
   int depth = 0, peer = 0, diver = 0;
   struct replex root = { 'm', NULL, NULL, NULL, NULL };
   struct replex* iter = &root;
   struct replex* prev = NULL;

   f = fopen( "repl.lex", "r" );

   while( NULL != fgets( buffer, 4096, f ) ) {
      iter = &root;
      for( depth = 0 ; strlen( buffer ) > depth ; depth++ ) {
         peer = 0;

         while( 1 ) {
            if( '\0' == iter->c ) {
               /* This is an empty node. */
               if( diver == depth ) {
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
               printf( "%c\n", iter->c );
               if( NULL == iter->peer_up ) {
                  iter->peer_up = calloc( 1, sizeof( struct replex ) );
               }

               prev = iter;
               iter = iter->peer_up;
               continue;

            } else {
               if( NULL == iter->peer_down ) {
                  iter->peer_down = calloc( 1, sizeof( struct replex ) );
               }
               prev = iter;
               iter = iter->peer_down;
               continue;
            }

            break;
         }
      }
   }

   fclose( f );

   return 0;
}

