
#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#include "mem.h"

typedef uint16_t BITFIELD;

typedef int16_t MLEN_T;
typedef uint8_t MEM_ID;

struct mvar {
   uint8_t pid;
   MEM_ID mid;
   MLEN_T len;  /* Used. */
   MLEN_T size; /* Allocated. */
   uint8_t data[];
} __attribute__((packed));

#define MEM_HEAP_SIZE 420

#ifdef CHECK
void mshift( MLEN_T start, MLEN_T offset );
#endif /* CHECK */

#if defined( MPRINT ) || defined( CHECK )
void mprint();
#endif /* MPRINT || CHECK */

void minit();
void* mget( uint8_t pid, MEM_ID mid, MLEN_T sz );
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

