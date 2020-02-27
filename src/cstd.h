
#ifndef CSTD_H
#define CSTD_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define printf( ... ) fprintf( NULL, __VA_ARGS__ )
#define isprintable( c ) (' ' <= c && '~' >= c)
#define isdigit( c ) ('0' <= (c) && '9' >= (c))
#define islower( c ) ('a' <= (c) && 'z' >= (c))
#define isupper( c ) ('A' <= (c) && 'Z' >= (c))

typedef void FILE;

void* malloc( size_t sz ); 
void free( void* ptr );
void* memset( void* ptr, int c, size_t sz );
void* memcpy( void* dest, const void* src, size_t sz );

size_t strlen( const char* str );
int strncmp( const char* str1, const char* str2, size_t sz );
char* strtok( char* str, size_t sz, const char* delim );
unsigned int atou( const char* str, int base );
char* utoa( unsigned int num, char* dest, int base );
size_t udigits( unsigned int num, int base );

void fprintf( FILE* f, const char* pattern, ... );
void putc( char c, FILE* f );

#endif /* CSTD_H */

