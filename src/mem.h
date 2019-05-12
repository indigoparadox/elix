
#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define mem_alloc( count, size) calloc( count, size )
#define mem_free( ptr ) if( NULL != ptr ) { free( ptr ); }

#define do_error( msg ) perror( msg )

#endif /* MEM_H */

