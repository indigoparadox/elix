
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
#define isalnum( c ) \
	(isdigit( c ) || \
	islower( c ) || \
	isupper( c ))

typedef void FILE;

void* malloc( size_t sz ); 
void free( void* ptr );
void* memset( void* ptr, int c, size_t sz );
void* memcpy( void* dest, const void* src, size_t sz );

size_t strlen( const char* str );
int strncmp( const char* str1, const char* str2, size_t sz );
char* strtok( char* str, size_t sz, const char* delim );
void strnreplace( char* str, size_t sz, char* s, char* r );
void strmtrunc( char* str, size_t start, size_t diff );
unsigned int atou( const char* str, int base );
char* utoan( unsigned int num, char* dest, size_t dest_sz, int base );
size_t udigits( unsigned int num, int base );

void fprintf( FILE* f, const char* pattern, ... );
void putc( char c, FILE* f );
#ifdef CSTD_PUTCHAR
int putchar( int c );
#endif /* CSTD_PUTCHAR */

#endif /* CSTD_H */

