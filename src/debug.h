
#ifndef DEBUG_H
#define DEBUG_H

#ifdef SILENT

#define derror( msg )
#define deprintf( msg, ... )
#define dinfo( msg )
#define diprintf( msg, ... )
#define ddebug( msg )
#define ddprintf( msg, ... )

#else

#include <stdio.h>

#define derror( msg ) fprintf( stderr, msg )
#define deprintf( msg, ... ) fprintf( stderr, msg, __VA_ARGS__ )
#define dinfo( msg ) fprintf( stdout, msg )
#define diprintf( msg, ... ) fprintf( stdout, msg, __VA_ARGS__ )
#ifdef DEBUG
#define ddebug( msg ) fprintf( stdout, msg )
#define ddprintf( msg, ... ) fprintf( stdout, msg, __VA_ARGS__ )
#else
#define ddebug( msg ) 
#define ddprintf( msg, ... )
#endif /* DEBUG */

#endif /* SILENT */

#endif /* DEBUG_H */

