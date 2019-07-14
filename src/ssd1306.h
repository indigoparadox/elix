
#ifndef SSD1306_H
#define SSD1306_H

/*! \file ssd1306.h
 *  \brief A platform-independent driver for the SSD1306 OLED display.
 *         Requires I2C support.
 */

#include "i2c.h"

#define SSD1306_LCDWIDTH 				128
#define SSD1306_LCDHEIGHT 				64
#define SSD1306_MAXROWS 				7
#define SSD1306_MAXCONTRAST 			0xFF

/* Command table. */
#define SSD1306_SETCONTRAST 			0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 			0xA5
#define SSD1306_NORMALDISPLAY 		0xA6
#define SSD1306_INVERTDISPLAY 		0xA7
#define SSD1306_DISPLAYOFF 			0xAE
#define SSD1306_DISPLAYON 				0xAF

/* Scrolling commands. */
#define SSD1306_SCROLL_RIGHT 			0x26
#define SSD1306_SCROLL_LEFT 			0X27
#define SSD1306_SCROLL_VERT_RIGHT	0x29
#define SSD1306_SCROLL_VERT_LEFT 	0x2A
#define SSD1306_SET_VERTICAL 			0xA3

/* Address setting. */
#define SSD1306_SETLOWCOLUMN 			0x00
#define SSD1306_SETHIGHCOLUMN 		0x10
#define SSD1306_MEMORYMODE 			0x20
#define SSD1306_COLUMNADDRESS 		0x21
#define SSD1306_COLUMNADDRESS_MSB	0x00
#define SSD1306_COLUMNADDRESS_LSB 	0x7F
#define SSD1306_PAGEADDRESS 			0x22
#define SSD1306_PAGE_START_ADDRESS 	0xB0

/* Hardware configuration. */
#define SSD1306_SETSTARTLINE 			0x40
#define SSD1306_SEGREMAP 				0xA1
#define SSD1306_SETMULTIPLEX 			0xA8
#define SSD1306_COMSCANINC 			0xC0
#define SSD1306_COMSCANDEC 			0xC8
#define SSD1306_SETDISPLAYOFFSET 	0xD3
#define SSD1306_SETCOMPINS 			0xDA

/* Timing and driving. */
#define SSD1306_SETDISPLAYCLOCKDIV 	0xD5
#define SSD1306_SETPRECHARGE 			0xD9
#define SSD1306_SETVCOMDETECT 		0xDB
#define SSD1306_NOP 						0xE3

/* Power supply configuration. */
#define SSD1306_CHARGEPUMP 			0x8D
#define SSD1306_EXTERNALVCC 			0x10
#define SSD1306_SWITCHCAPVCC 			0x20

#define SSD1306_ADDRESS 				0x3C

/* Default values. */
#ifndef SSD1306_VAL_CONTRAST
#define SSD1306_VAL_CONTRAST 0x07
#endif /* !SSD1306_VAL_CONTRAST */

void ssd1306_init();

#endif /* SSD1306_H */

