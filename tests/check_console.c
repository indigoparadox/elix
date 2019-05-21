
#include <check.h>
#include <stdint.h>
#include "../src/console.h"
#include "../src/alpha.h"

#define CHECK_DISPLAY_BUFFER_LEN 100

uint8_t g_system_state = 0;

char g_display_buffer[CHECK_DISPLAY_BUFFER_LEN] = { 0 };
int g_display_idx = 0;

const struct astring g_str_intc_pattern = astring_l( "Test pattern: %d, %c\n" );
const struct astring g_str_int_pattern = astring_l( "Test pattern: %d\n" );
const struct astring g_str_int4_pattern = astring_l( "Test pattern: %4d\n" );
const struct astring g_str_hexc_pattern = astring_l( "Test pattern: %x, %c\n" );
const struct astring g_str_hex_pattern = astring_l( "Test pattern: %x\n" );
const struct astring g_str_hex4_pattern = astring_l( "Test pattern: %4x\n" );

void display_init() {
   memset( g_display_buffer, '\0', CHECK_DISPLAY_BUFFER_LEN );
   g_display_idx = 0;
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

START_TEST( test_console_putc ) {
   display_init();

   display_putc( 'a' );

   ck_assert_int_eq( g_display_buffer[0], 'a' );
}
END_TEST

START_TEST( test_console_printf_int ) {
   int test_int = 543;
   char test_char = 'a';

   display_init();

   tprintf( &g_str_intc_pattern, test_int, test_char );
   ck_assert_str_eq( g_display_buffer, "Test pattern: 543, a\n" );

   display_init();

   tprintf( &g_str_int_pattern, test_int );
   ck_assert_str_eq( g_display_buffer, "Test pattern: 543\n" );

   display_init();

   tprintf( &g_str_int4_pattern, test_int );
   ck_assert_str_eq( g_display_buffer, "Test pattern: 0543\n" );
}
END_TEST

START_TEST( test_console_printf_hex ) {
   int test_int = 0x543;
   char test_char = 'a';

   display_init();

   tprintf( &g_str_hexc_pattern, test_int, test_char );
   ck_assert_str_eq( g_display_buffer, "Test pattern: 543, a\n" );

   display_init();

   tprintf( &g_str_hex_pattern, test_int );
   ck_assert_str_eq( g_display_buffer, "Test pattern: 543\n" );

   display_init();

   tprintf( &g_str_hex4_pattern, test_int );
   ck_assert_str_eq( g_display_buffer, "Test pattern: 0543\n" );
}
END_TEST

Suite* console_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "console" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_console_putc );
   tcase_add_test( tc_core, test_console_printf_int );
   tcase_add_test( tc_core, test_console_printf_hex );

   suite_add_tcase( s, tc_core );

   return s;
}

