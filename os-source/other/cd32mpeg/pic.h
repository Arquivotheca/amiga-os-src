/* pic.h - Definitions related to the Z2 and CDGS MPEG Cards */

#ifndef	PIC_H
#define PIC_H

#include <exec/types.h>

/* AutoConfig Stuff */

#define	Z2_PROTO_MANUFACTURER	0x202
#define	Z2_PROTO_PRODUCT	0x68

/* Board Type */

#define Z2_PROTO	0x01
#define CDGS_PROTO	0x02

/* ZorroII Control Register Bits */

/* Write Bits */
#define USE_ADSP2105   0x8000
#define LSTARTI        0x4000
#define LHDMUTE        0x2000
#define MUTE           0x1000
#define DACINIT        0x0800
#define DACLATCH       0x0400
#define DACATT         0x0200
#define DACSHIFT       0x0100
#define DACSOC_MASK    0x00C0
#define CINTACK_N      0x0010
#define VIDEO_DISABLE  0x0004		/* When this bit is set MPEG video will be passed when
					   the output color is in the following color space
					   11xxxxxx111xxxxx011xxxxx. */
#define KEY1           0x0002
#define KEY0           0x0001

#define COND_GENLOCK   0x0003		/* The output genlock signal is the Amiga Pixel Switch
					   ANDed with the NOT of the lowest order bit of the high
					   high order nybble of the Amiga blue. */

#define PASS_GENLOCK   0x0002		/* The output genlock signal is the Amiga Pixel Switch. */
#define ALWAYS_GENLOCK 0x0001		/* The genlock signal is always asserted. */
#define NEVER_GENLOCK  0x0000		/* The genlock signal is never asserted. */


/* ... Reads */
#define CINT_N    0x0080
#define LINT_N    0x0040
#define DMS       0x0020
//#define CINTACK_N 0x0010			This is duplicated in the write list.
#define CFLEVEL   0x0008
#define LFALF_N   0x0004
#define LFALE_N   0x0002
#define LFEMPTY_N 0x0001

/* ZorroII Board Layout */

#define Z2_CL450_OFFSET 0x00200000L
#define Z2_CL450_GCLK_FREQ 40000000.0
#define Z2_CL450_GCLK_PERIOD_NS (1000000000.0/MPEG_CL450_GCLK_FREQ)
#define Z2_CL450_DRAM_REFRESH_NS 4000000
#define Z2_CL450_DRAM_ROWS 512
#define Z2_L64111_OFFSET 0x00100000L

#endif	/* PIC_H */