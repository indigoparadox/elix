
#include <check.h>
#include <stdint.h>
#include "../src/console.h"

#define CHECK_DISPLAY_BUFFER_LEN 100

uint8_t g_system_state = 0;

char g_display_buffer[CHECK_DISPLAY_BUFFER_LEN] = { 0 };
int g_display_idx = 0;

void display_init() {
   memset( g_display_buffer, '\0', CHECK_DISPLAY_BUFFER_LEN );
}

void display_putc( char c ) {
   g_display_buffer[g_display_idx] = c;
   g_display_idx++;
}

int keyboard_hit() {
   return 0;
}

unsigned char keyboard_getc() {
   return '\0';
}

START_TEST( test_console_printf ) {
   int test_int = 543;
   char test_char = 'a';

   display_init();

   tprintf( "Test pattern: %d, %c\n", test_int, test_char );

   ck_assert_str_eq( g_display_buffer, "Test pattern: 543, a\n" );
}
END_TEST

Suite* console_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "console" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_console_printf );

   suite_add_tcase( s, tc_core );

   return s;
}

