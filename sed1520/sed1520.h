/*-
 * Copyright (c) 2013 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/* SED1520 Command set */
#define DISPLAY_ON		0xAF
#define DISPLAY_OFF		0xAE
#define DISPLAY_START_LINE	0xC0
#define SET_PAGE_ADDRESS	0xB8
#define SET_COLUMN_ADDRESS	0x00
#define ADC_FORWARD		0xA0
#define ADC_REVERSE		0xA1
#define STATIC_DRIVE_ON		0xA5
#define STATIC_DRIVE_OFF	0xA4
#define DUTY_RATIO_16		0xA8
#define DUTY_RATIO_32		0xA9
#define READ_MODIFY_WRITE	0xE0
#define END_READ_MODIFY		0xEE
#define RESET			0xE2

/* Status flags */
#define STATUS_BUSY 0x80
#define STATUS_RESET 0x10

/* Controller bits */
#define CTRL1 0x01
#define CTRL2 0x02

/* Display size */
#define GLCD_XPIXELS	122
#define GLCD_YPIXELS	32

void glcdBusyWait(unsigned char, unsigned char);
void glcdCommandWrite(unsigned char, unsigned char);
void glcdSetAddress(unsigned char, unsigned char);
void glcdDataWrite(unsigned char);
unsigned char glcdDataRead(void);
void glcdClearScreen(void);
void glcdInit(void);
