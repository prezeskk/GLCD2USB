/**
 * PIN definitions for the SED1520 driver.
 */

/* Data port: All pins must be on the same port */
#define SED1520_DATA_PORT 	PORTC
#define SED1520_DATA_DDR 	DDRC
#define SED1520_DATA_PIN 	PINC

/* Control port: All pins must be on the same port */
#define SED1520_CONTROL_PORT 	PORTA
#define SED1520_CONTROL_DDR 	DDRA

/* Common control lines */
#define SED1520_A0 (1 << 7)
#define SED1520_RW (1 << 6)
#define SED1520_RES (1 << 0)

/* Depending on chip type */
#ifdef HAS_CS_LINES
# define SED1520_CS1 (1 << 4)
# define SED1520_CS2 (1 << 3)
# define SED1520_E (1 << 5)
#else
# define SED1520_E1 (1 << 4)
# define SED1520_E2 (1 << 3)
#endif
