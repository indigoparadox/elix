
#ifndef CSTD_H
#define CSTD_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef void FILE;

void* malloc( size_t sz ); 
void free( void* ptr );
void* memset( void* ptr, int c, size_t sz );
void* memcpy( void* dest, const void* src, size_t sz );

size_t strlen( const char* str );
int strncmp( const char* str1, const char* str2, size_t sz );
unsigned int atou( const char* str );
char* utoa( unsigned int num, char* dest, int base );
size_t udigits( unsigned int num, int base );

void fprintf( FILE* f, const char* pattern, ... );
void putc( char c, FILE* f );

#endif /* CSTD_H */

