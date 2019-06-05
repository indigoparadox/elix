
#ifndef MEM_H
#define MEM_H

#include <stdint.h>

typedef int16_t MEMLEN_T;
typedef uint16_t BITFIELD;
typedef uint8_t MEM_ID;

#define MGET_UNSET      -1
#define MGET_NO_CREATE  0

struct mvar {
   uint8_t pid;
   MEM_ID mid;
   MEMLEN_T size; /* Allocated. */
   uint8_t data[];
} __attribute__((packed));

#define MEM_HEAP_SIZE 420

#ifdef CHECK
void mshift( MEMLEN_T start, MEMLEN_T offset );
#endif /* CHECK */

#if defined( MPRINT ) || defined( CHECK )
void mprint();
#endif /* MPRINT || CHECK */

#ifdef CHECK
int mget_pos( int pid, int mid );
#endif /* CHECK */

void minit();
void* mget( uint8_t pid, MEM_ID mid, MEMLEN_T sz );
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

