
#ifndef MEM_H
#define MEM_H

#include <stdint.h>

typedef uint8_t MEMLEN_T;

typedef uint16_t BITFIELD;

struct mvar {
   uint8_t pid;
   uint8_t mid;
   uint16_t len;  /* Used. */
   uint16_t size; /* Allocated. */
   uint8_t data[];
} __attribute__((packed));

#define MEM_HEAP_SIZE 420

#define mget_ptr( pid, mid, src, type ) \
   (*((type*)(mget( pid, mid, src, sizeof( void* ) ))))
#define mget_int( pid, mid ) \
   (*((int*)(mget( pid, mid, NULL, sizeof( int ) ))))
#define mget_len( pid, mid ) \
   (*((MEMLEN_T*)(mget( pid, mid, NULL, sizeof( MEMLEN_T ) ))))

#ifdef CHECK
void mshift( int start, int offset );
#endif /* CHECK */

#if defined( MPRINT ) || defined( CHECK )
void mprint();
#endif /* MPRINT || CHECK */

void minit();
void* mget( int pid, int mid, void* src, MEMLEN_T sz );
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

