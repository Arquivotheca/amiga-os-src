/*
 *
 * anim.h:      Definitions for IFFParse ANIM reader.
 *
 * 12/14/91
 */

#ifndef IFFP_ANIM_H
#define IFFP_ANIM_H

#ifndef IFFP_IFF_H
#include "iffp/iff.h"
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif
#ifndef GRAPHICS_VIDEOCONTROL_H
#include <graphics/videocontrol.h>
#endif

#ifndef NO_PROTOS
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#endif

/*  IFF types we may encounter  */
#define ID_ANIM         MAKE_ID('A','N','I','M')

/* ILBM Chunk ID's we may encounter
 * (see iffp/iff.h for some other generic chunks)
 */
#define ID_ANHD         MAKE_ID('A','N','H','D')
#define ID_DLTA         MAKE_ID('D','L','T','A')

/* ---------- ANimationHeaDer ----------------------------------------*/
/*  Required ANHD structure describes an ANIM frame */
typedef struct {
	UBYTE	operation;	/*  The compression method:
				    =0 set directly (normal ILBM BODY),
				    =1 XOR ILBM mode,
				    =2 Long Delta mode,
				    =3 Short Delta mode,
				    =4 Generalized short/long Delta mode,
				    =5 Byte Vertical Delta mode
				    =6 Stereo op 5 (third party)
				    =74 (ascii 'J') reserved for Eric Graham's
				        compression technique (details to be
				        released later). */
	UBYTE	mask;		/* (XOR mode only - plane mask where each
				    bit is set =1 if there is data and =0
				    if not.) */
	UWORD	w,h;		/* (XOR mode only - width and height of the
				    area represented by the BODY to eliminate
				    unnecessary un-changed data) */
	WORD	x,y;		/* (XOR mode only - position of rectangular
				   area representd by the BODY) */
	ULONG	abstime;	/* (currently unused - timing for a frame
				   relative to the time the first frame
				   was displayed - in jiffies (1/60 sec)) */
	ULONG	reltime;	/* (timing for frame relative to time
				   previous frame was displayed - in
				   jiffies (1/60 sec)) */
	UBYTE	interleave;	/* (unused so far - indicates how may frames
				   back this data is to modify.  =0 defaults
				   to indicate two frames back (for double
				   buffering). =n indicates n frames back.
				   The main intent here is to allow values
				   of =1 for special applications where
				   frame data would modify the immediately
				   previous frame.) */
	UBYTE	pad0;		/* Pad byte, not used at present. */
	ULONG	bits;		/* 32 option bits used by options=4 and 5.
				   At present only 6 are identified, but the
				   rest are set =0 so they can be used to
				   implement future ideas.  These are defined
				   for option 4 only at this point.  It is
				   recommended that all bits be set =0 for
				   option 5 and that any bit settings
				   used in the future (such as for XOR mode)
				   be compatible with the option 4
				   bit settings.   Player code should check
				   undefined bits in options 4 and 5 to assure
				   they are zero.

				   The six bits for current use are:

				    bit #              set =0               set =1
				    ===============================================
				    0              short data           long data
				    1                 set                  XOR
				    2             separate info        one info list
						  for each plane       for all planes
				    3               not RLC        RLC (run length coded)
				    4              horizontal           vertical
				    5           short info offsets   long info offsets
				*/

	UBYTE	pad[16];	/* This is a pad for future use for future
				   compression modes. */
} AnimHeader;

#ifndef NO_PROTOS

/* animr.c */
LONG loaddlta(struct IFFHandle *iff, struct BitMap *bitmap, AnimHeader *anhd,
		LONG *dltabuffer);
BOOL make_ytable(WORD width, WORD height);
void free_ytable(void);

/* unvscomp.asm */
decode_vkplane(char *in, char *out, WORD linebytes);

#endif /* NO_PROTOS */

#endif /* IFFP_ANIM_H */
