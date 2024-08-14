/*
	Internal defines for the printer device.
*/

#define PRT_BW			0x01	/* non-color picture */
#define PRT_BLACKABLE		0x02	/* printer has black capabilities */
#define PRT_BW_BLACKABLE	0x03	/* combination of above two */
#define PRT_HAM			0x04	/* printing a ham picture */
#define PRT_INVERT		0x08	/* invert picture */
#define PRT_NOBLIT		0x10	/* no use blitter in ReadPixelLine */
#define PRT_RENDER0		0x20	/* render (case 0) has been called */
#define PRT_NORPL		0x40	/* can't use ReadPixelLine */
#define PRT_BELOW		0x80	/* there is a line below us */

#define SPECIAL_FIX_RGB_MASK \
	(SPECIAL_FIX_RED | SPECIAL_FIX_GREEN | SPECIAL_FIX_BLUE)

#define MAXBLITSIZE	1008	/* max pixels blitter can xfer at 1 time */
#define MAXDEPTH	8	/* max planes ClipBlit can blit at 1 time */
