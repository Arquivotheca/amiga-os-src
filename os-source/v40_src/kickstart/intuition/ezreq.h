
/*** ez.h *****************************************************************
 *
 *  EZRequester layout "constants" and IntuiText List structure
 *
 *  $Id: ezreq.h,v 38.1 92/02/11 13:37:22 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

/*
 * this thing maintains an array of IntuiText structures.
 * They come back linked from create routines ( formatITList )
 * but you can delink them and still free them all at once
 * (in fact, you can *only* free them all at once).
 */
struct ITList {
    struct IntuiText	*itl_IText;
    UWORD		itl_NumIText;
    UBYTE		*itl_Buffer;
    UWORD		itl_BufferSize;
    UWORD		*itl_NextArg;		/* not using this */
};

/*
 * this is the userdata for a system requester window,
 * used to track the things we need to free.
 *
 * we free:
 * gadgets from ReqGadget list
 * images from ReqImage
 * frame image from SysReqTrack
 * everything else via FreeRemember, including the SysReqTrack
 *  structure, which should therefore go LAST.
 */

struct SysReqTrack {
    struct Remember	*srt_Remember;
    void		*srt_GadgetFrame;
    struct Gadget	*srt_ReqGadgets;
    struct Gadget	*srt_ImageGad;
};

/* interim constants for layout quantities */

/* the set of constants here that should be res-sensitive:
 *	SPACING1
 *	GADSPACE
 *	HBIGSPACE
 *	VBIGSPACE
 *	xxxEXTRAGWIDTH	* font-sensitive?	
 *	xxxEXTRAGHEIGHT	* font-sensitive?
 *	TEXTHSPACE
 */

#if 0	/* using res-sensitive quantities	*/
#define SPACING1	(6)	/* vspace between body and gadgets	*/
#define GADSPACE	(12)	/* hspace between gadgets		*/
#define HBIGSPACE	(10)	/* requester space to left/right
				 * of frame/gadgets
				 */
#define VBIGSPACE	(4)	/* space above frame, below gadgets	*/
#define TEXTHSPACE	(20)	/* left text margin in frame	*/
#endif

#if 0	/* unused */
#define LEADING		(4)	/* unused now: using algoritmic val	*/
#define EXTRAGWIDTH	(8)	/* xtra space between gtext and gframe	*/
#define EXTRAGHEIGHT	(2)
#define GADHMARGIN	(5)	/* unused	*/
#define GADVMARGIN	(2)	/* unused	*/
#endif

#define GIMAGECOLOR	(2)	/* unused	*/
#define GTEXTCOLOR	(AUTOBACKPEN)	/* unused	*/

#define SEPARATOR	'|'
#define REQIFLAGS	(GADGETUP | VANILLAKEY)
#define SYSREQBACKFILL (0)

#define EZMINWIDTH	(140)
#define EZMINHEIGHT	(88)

/* don't expect any bodytext at this position */
#define MAGIC_LAYOUT_VAL	(-229)

#define printf kprintf
