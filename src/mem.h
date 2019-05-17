
#ifndef MEM_H
#define MEM_H

#include <stdint.h>

struct mvar {
   uint8_t pid;
   uint8_t mid;
   uint16_t len;  /* Used. */
   uint16_t size; /* Allocated. */
} __attribute__((packed));

#define MEM_HEAP_SIZE 420

#define mget_ptr( pid, mid, psz, type ) \
   (*((type*)(mget( pid, mid, psz ))))
#define mget_int( pid, mid ) \
   (*((int*)(mget( pid, mid, NULL ))))

#ifdef CHECK
void mshift( int start, int offset );
#endif /* CHECK */

#if defined( MPRINT ) || defined( CHECK )
void mprint();
#endif /* MPRINT || CHECK */

void minit();
void mset( int pid, int mid, void* ptr, int len );
void* mget( int pid, int mid, int* psz );
void mzero( void* dest, int sz );
int mcopy( void* dest, const void* src, int sz );
int mcompare( const void* c1, const void* c2, int sz );

//struct mstring* mfromcstr( char* str );

#ifdef MEM_C
const long meta_null = 0;
#else
extern const long meta_null;
#endif /* MEM_C */

#endif /* MEM_H */

