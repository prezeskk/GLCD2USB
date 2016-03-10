/**
 * Driver for graphical display using a SED1520 or compatible controller.
 *
 * Displays using a SED1520 usually come in one of two configurations:
 *
 * A. The display uses SED1520 with external clock. Those have a CS1 and CS2
 *    (chip select) line and share A0, E, R/W lines.
 * B. The display uses SED1520 with internal oscillator. These displays do
 *    not need an external clock. They can be identified by a small resistor
 *    next to one chip (likely labeled RF). As the chips do not have a CS line
 *    in this configuration, usually the enables lines of each one is available
 *    as E1 and E2.
 *
 * The display I use most is of type B and the first code has been written for
 * it. If you use a type A LCD when set option -DHAS_CS_LINES in the Makefile.
 *
 * Timings used herein:
 *
 * - The minimum Enable pulse width is 80 ns. On an ATmega running at 20 MHz
 *   one instruction execution time is ~50 ns. Therefore two NOPs are used to
 *   time Enable pins.
 * - The driver can use busy check if compiled with -DBUSYCHECK. I found that
 *   it does work even without and updating is much faster.
 */

/*-
 * Copyright (c) 2013 Markus Dolze
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "sed1520.h"
#include "sed1520conf.h"

unsigned char lcd_x = 0, lcd_page = 0;

/**
 * Setup AVR ports.
 */
static void
glcdInitHW(void)
{
	SED1520_DATA_DDR = 0xFF;
#ifdef HAS_CS_LINES
	SED1520_CONTROL_DDR |= (SED1520_CS1 | SED1520_CS2 | SED1520_E | SED1520_RW | SED1520_A0 | SED1520_RES);
#else
	SED1520_CONTROL_DDR |= (SED1520_E1 | SED1520_E2 | SED1520_RW | SED1520_A0 | SED1520_RES);
#endif
	asm("nop");
	/* Reset pulse */
	SED1520_CONTROL_PORT |= SED1520_RES;
	_delay_ms(2);
	SED1520_CONTROL_PORT &= ~SED1520_RES;
	_delay_ms(2);
	/* Reset = high = 68 family MPU */
	SED1520_CONTROL_PORT |= SED1520_RES;
}

/**
 * Initialize display.
 */
void
glcdInit(void)
{
	glcdInitHW();
	glcdCommandWrite(RESET, CTRL1 + CTRL2);
#ifdef BUSYCHECK
	glcdBusyWait(STATUS_RESET, CTRL1 + CTRL2);
#endif
	glcdCommandWrite(DISPLAY_ON, CTRL1 + CTRL2);
	glcdCommandWrite(DISPLAY_START_LINE | 0, CTRL1 + CTRL2);
}

#ifdef BUSYCHECK
/**
 * Wait for display controller to be ready. Only one controller may be checked
 * at a time.
 */
void
glcdBusyWait(unsigned char statusbit, unsigned char controller)
{
	unsigned char tmp;

	if (controller == 0)
		return;

	SED1520_CONTROL_PORT &= ~SED1520_A0;
	SED1520_CONTROL_PORT |= SED1520_RW;

	SED1520_DATA_DDR = 0x00;
	SED1520_DATA_PORT = 0xFF;

	do {
#ifdef HAS_CS_LINES
		if (controller & CTRL1)
			SED1520_CONTROL_PORT &= ~SED1520_CS1;
		if (controller & CTRL2)
			SED1520_CONTROL_PORT &= ~SED1520_CS2;
		asm("nop");
		SED1520_CONTROL_PORT |= SED1520_E;
		asm("nop");
		asm("nop");
		tmp = SED1520_DATA_PIN;
		SED1520_CONTROL_PORT &= ~SED1520_E;
		SED1520_CONTROL_PORT |= (SED1520_CS1 | SED1520_CS2);
#else
		if (controller & CTRL1) {
			SED1520_CONTROL_PORT |= SED1520_E1;
			asm("nop");
			asm("nop");
			tmp = SED1520_DATA_PIN;
			SED1520_CONTROL_PORT &= ~SED1520_E1;
		}
		else {
			SED1520_CONTROL_PORT |= SED1520_E2;
			asm("nop");
			asm("nop");
			tmp = SED1520_DATA_PIN;
			SED1520_CONTROL_PORT &= ~SED1520_E2;
		}
#endif
	} while (tmp & statusbit);

	SED1520_DATA_DDR = 0xFF;
}
#endif

/**
 * Write command.
 */
void
glcdCommandWrite(unsigned char commandToWrite, unsigned char controller)
{
#ifdef BUSYCHECK
	glcdBusyWait(STATUS_BUSY, controller & CTRL1);
	glcdBusyWait(STATUS_BUSY, controller & CTRL2);
#endif

	SED1520_CONTROL_PORT &= ~SED1520_A0;
	SED1520_CONTROL_PORT &= ~SED1520_RW;

	SED1520_DATA_PORT = commandToWrite;

#ifdef HAS_CS_LINES
	if (controller & CTRL1)
		SED1520_CONTROL_PORT &= ~SED1520_CS1;
	if (controller & CTRL2)
		SED1520_CONTROL_PORT &= ~SED1520_CS2;
	asm("nop");
	SED1520_CONTROL_PORT |= SED1520_E;
	asm("nop");
	asm("nop");
	SED1520_CONTROL_PORT &= ~SED1520_E;
	SED1520_CONTROL_PORT |= (SED1520_CS1 | SED1520_CS2);
#else
	if (controller & CTRL1) {
		SED1520_CONTROL_PORT |= SED1520_E1;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT &= ~SED1520_E1;
	}
	if (controller & CTRL2) {
		SED1520_CONTROL_PORT |= SED1520_E2;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT &= ~SED1520_E2;
	}
#endif
}

/**
 * Write data.
 */
void
glcdDataWrite(unsigned char dataToWrite)
{
#ifdef BUSYCHECK
	glcdBusyWait(STATUS_BUSY, CTRL1);
	glcdBusyWait(STATUS_BUSY, CTRL2);
#endif

	SED1520_CONTROL_PORT |= SED1520_A0;
	SED1520_CONTROL_PORT &= ~SED1520_RW;

	SED1520_DATA_PORT = dataToWrite;

	/* Update either left or right half of the display */
#ifdef HAS_CS_LINES
	if (lcd_x < (GLCD_XPIXELS / 2))
		SED1520_CONTROL_PORT &= ~SED1520_CS1;
	else
		SED1520_CONTROL_PORT &= ~SED1520_CS2;
	asm("nop");
	SED1520_CONTROL_PORT |= SED1520_E;
	asm("nop");
	asm("nop");
	SED1520_CONTROL_PORT &= ~SED1520_E;
	SED1520_CONTROL_PORT |= (SED1520_CS1 | SED1520_CS2);
#else
	if (lcd_x < (GLCD_XPIXELS / 2)) {
		SED1520_CONTROL_PORT |= SED1520_E1;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT &= ~SED1520_E1;
	}
	else {
		SED1520_CONTROL_PORT |= SED1520_E2;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT &= ~SED1520_E2;
	}
#endif

	/* Increase internal counter and wrap page if end of page reached */
	lcd_x++;
	if (lcd_x >= GLCD_XPIXELS)
		glcdSetAddress(0, lcd_page + 1);
}

/**
 * Read data.
 */
unsigned char
glcdDataRead(void)
{
	unsigned char tmp;

#ifdef BUSYCHECK
	glcdBusyWait(STATUS_BUSY, CTRL1);
	glcdBusyWait(STATUS_BUSY, CTRL2);
#endif

	SED1520_CONTROL_PORT |= SED1520_A0;
	SED1520_CONTROL_PORT |= SED1520_RW;

	SED1520_DATA_DDR = 0x00;
	SED1520_DATA_PORT = 0xFF;

	/* Read from either left or right half of the display */
#ifdef HAS_CS_LINES
	if (lcd_x < (GLCD_XPIXELS / 2))
		SED1520_CONTROL_PORT &= ~SED1520_CS1;
	else
		SED1520_CONTROL_PORT &= ~SED1520_CS2;
	asm("nop");
	/* Dummy read */
	SED1520_CONTROL_PORT |= SED1520_E;
	asm("nop");
	asm("nop");
	SED1520_CONTROL_PORT &= ~SED1520_E;
	asm("nop");
	asm("nop");
	/* Really read */
	SED1520_CONTROL_PORT |= SED1520_E;
	asm("nop");
	asm("nop");
	tmp = SED1520_DATA_PIN;
	SED1520_CONTROL_PORT &= ~SED1520_E;
	SED1520_CONTROL_PORT |= (SED1520_CS1 | SED1520_CS2);
#else
	if (lcd_x < (GLCD_XPIXELS / 2)) {
		/* Dummy read */
		SED1520_CONTROL_PORT |= SED1520_E1;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT &= ~SED1520_E1;
		asm("nop");
		asm("nop");
		/* Really read */
		SED1520_CONTROL_PORT |= SED1520_E1;
		asm("nop");
		asm("nop");
		tmp = SED1520_DATA_PIN;
		SED1520_CONTROL_PORT &= ~SED1520_E1;
	}
	else {
		SED1520_CONTROL_PORT |= SED1520_E2;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT &= ~SED1520_E2;
		asm("nop");
		asm("nop");
		SED1520_CONTROL_PORT |= SED1520_E2;
		asm("nop");
		asm("nop");
		tmp = SED1520_DATA_PIN;
		SED1520_CONTROL_PORT &= ~SED1520_E2;
	}
#endif

	SED1520_DATA_DDR = 0xFF;

	/* Increase internal counter and wrap page if end of page reached */
	lcd_x++;
	if (lcd_x >= GLCD_XPIXELS)
		glcdSetAddress(0, lcd_page + 1);

	return tmp;
}

/**
 * Set display column (x) and page (p) address.
 */
void
glcdSetAddress(unsigned char x, unsigned char p)
{
	lcd_x = x;
	lcd_page = p;

	/*
	 * If X is in left half of display, set column on first controller and
	 * set column 0 on second controller. Page is the same for both.
	 * Otherwise only set column and page on the second controller only.
	 */
	if (x < (GLCD_XPIXELS / 2)) {
		glcdCommandWrite(SET_COLUMN_ADDRESS | lcd_x, CTRL1);
		glcdCommandWrite(SET_COLUMN_ADDRESS | 0, CTRL2);
		glcdCommandWrite(SET_PAGE_ADDRESS | lcd_page, CTRL1 + CTRL2);
	}
	else {
		glcdCommandWrite(SET_COLUMN_ADDRESS | (lcd_x - (GLCD_XPIXELS / 2)), CTRL2);
		glcdCommandWrite(SET_PAGE_ADDRESS | lcd_page, CTRL2);
	}
}

/**
 * Erase display content.
 */
void
glcdClearScreen(void)
{
	char j, i;
	for (j = 0; j < (GLCD_YPIXELS / 8); j++) {
		glcdSetAddress(0, j);
		for (i = 0; i < GLCD_XPIXELS; i++) {
			glcdDataWrite(0);
		}
	}
	glcdSetAddress(0, 0);
}
