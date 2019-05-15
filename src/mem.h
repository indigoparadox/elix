
#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#define MEM_HEAP_SIZE 300
#define MEM_BUFFER_SIZE 100

//#define mem_alloc( count, size) calloc( count, size )
//#define mem_free( ptr ) if( NULL != ptr ) { free( ptr ); }

void mset( int pid, int mid, void* ptr, int len );
void mget( int pid, int mid, void* buffer, int buffer_sz );
void mzero( void* dest, int sz );
int mcopy( void* dest, const void* src, int sz );
int mcompare( const void* c1, const void* c2, int sz );
int mstrlen( const char* str );

//struct mstring* mfromcstr( char* str );

#endif /* MEM_H */

