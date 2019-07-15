
#include "../ssd1306.h"

const uint8_t ssd1306_init_str[] = {
	0xAE,

	SSD1306_SETCONTRAST,
	SSD1306_VAL_CONTRAST,

   SSD1306_MEMORYMODE,
   SSD1306_MEMORYMODE_VERTICAL,

   SSD1306_COLUMNADDRESS,
   SSD1306_COLUMNADDRESS_MSB,
   SSD1306_COLUMNADDRESS_LSB,

   SSD1306_PAGEADDRESS,
	0x00,
	0x07,

   SSD1306_SETSTARTLINE,

	0xA0,

   SSD1306_SETMULTIPLEX,
	SSD1306_MULTIPLEX_MAX,

	0xC0,
	0xD3,
	0x00,
	0x8D,
	0x14,
	0xDA,
	0x12,
	0xD5,
	0x80,
	0xD9,
	0x22,
	0xDB,
	0x20,
	0xA6,
	0xA4,
	0xAF
};

const uint8_t length = sizeof( ssd1306_init_str );

void ssd1306_init() {
	int16_t i = 0;

   /* Open the connection. */
	i2c_start( 0x3c );

   /* Send the init sequence. */
	for( i = 0 ; length > i ; i++ ) {
		i2c_send_byte( 0x80 );
		i2c_send_byte( ssd1306_init_str[i] );
	}

   /* Start streaming mode. */
	i2c_send_byte( 0x00 );

   /* Light up some pixels. */
	for( i = 0 ; i < 500 ; i++ ) {
		i2c_send_byte( 0xff );
		i2c_send_byte( 0x00 );
	}

	i2c_stop();
}

