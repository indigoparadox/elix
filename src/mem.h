
#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#if 0
#define MSTR_CHUNK_SIZE 10

struct mstring {
   char* c_str;
   int len; /* Used. */
   int sz; /* Allocated. */
   struct mstring* next;
};
#endif

//#define mem_alloc( count, size) calloc( count, size )
//#define mem_free( ptr ) if( NULL != ptr ) { free( ptr ); }

void mzero( void* dest, int sz );
int mcopy( void* dest, const void* src, int sz );
int mcompare( const void* c1, const void* c2, int sz );
int mstrlen( const char* str );

//struct mstring* mfromcstr( char* str );

#endif /* MEM_H */

