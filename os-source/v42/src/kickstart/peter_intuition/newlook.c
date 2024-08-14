
/*** newlook.c ***************************************************************
 *
 *  newlook.c -- new embossed window borders and fancy system gadgets
 *
 *  $Id: newlook.c,v 38.11 93/01/14 14:30:25 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define DEMB(x)	;
#define DSG(x)	;	/* gadgets */

#include "intuall.h"
#include "classusr.h"
#include "gadgetclass.h"
#include "imageclass.h"
#include "newlook.h"
#include <graphics/gfxmacros.h>

#include "newlook_protos.h"

/*---------------------------------------------------------------------------*/

static struct Image * getScreenSysImage(struct Screen * s,
                                        int gindex);

/*---------------------------------------------------------------------------*/

	/* changes feb 8, 1990 */
/* "hard-coded" color scheme	*/
UWORD	fourColorPens[ NUMDRIPENS + 1] =  {
    0,		/* DETAILPEN	*/
    1,		/* BLOCKPEN	*/

    1,		/* TEXTPEN	*/
    2,		/* SHINEPEN	*/
    1,		/* SHADOWPEN	*/
    3,		/* FILLPEN	*/
    1,		/* FILLTEXTPEN	*/
    0,		/* BACKGROUNDPEN	*/
    2,		/* HIGHLIGHTTEXTPEN	*/
    1,		/* BARDETAILPEN	*/
    2,		/* BARBLOCKPEN	*/
    1,		/* BARTRIMPEN	*/

    (UWORD)~0,	/* terminator	*/
};


UWORD	twoColorPens[ NUMDRIPENS + 1] =  {
    0,		/* DETAILPEN	*/
    1,		/* BLOCKPEN	*/

    1,		/* TEXTPEN	*/
    1,		/* SHINEPEN	*/
    1,		/* SHADOWPEN	*/
    1,		/* FILLPEN	*/
    0,		/* FILLTEXTPEN	*/
    0,		/* BACKGROUNDPEN	*/
    1,		/* HIGHLIGHTTEXTPEN	*/
    0,		/* BARDETAILPEN	*/
    1,		/* BARBLOCKPEN	*/
    1,		/* BARTRIMPEN	*/

    (UWORD)~0,	/* terminator	*/
};


/* jump (TAG_MORE) to these after GA_IMAGE */
/* gadget width/height gotten from imag	*/

struct TagItem	sdragtags[] = {
    GA_LEFT,		0,
    GA_TOP,		0,
    GA_HEIGHT,		1,	/* will make taller */
    GA_RELWIDTH,	0,
    GA_SYSGTYPE,	SDRAGGING,
    TAG_END,
};

struct TagItem	wdragtags[] = {
    GA_LEFT,		0,
    GA_TOP,		0,
    GA_HEIGHT,		1,	/* will make taller */
    GA_RELWIDTH,	0,
    GA_SYSGTYPE,	WDRAGGING,
    TAG_END,
};

struct TagItem	closetags[] = {
    GA_LEFT,		0,
    GA_TOP,		0,
    GA_SYSGTYPE,	CLOSE,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};


struct TagItem	zoomtags[] = {
    GA_RELRIGHT,	ZOOMRELRIGHT,
    GA_TOP,		0,
    GA_SYSGTYPE,	WZOOM,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	depthtags[] = {
    GA_RELRIGHT,	DEPTHRELRIGHT,
    GA_TOP,		0,
    GA_SYSGTYPE,	WUPFRONT,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	sdepthtags[] = {
    GA_RELRIGHT,	DEPTHRELRIGHT,	/* aligned with window's depthg */
    GA_TOP,		0,
    GA_SYSGTYPE,	SUPFRONT,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	sizetags[] = {
    GA_RELRIGHT,	-17,
    GA_RELBOTTOM,	-9,
    GA_SYSGTYPE,	SIZING,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	lclosetags[] = {
    GA_LEFT,		0,
    GA_TOP,		0,
    GA_SYSGTYPE,	CLOSE,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	lzoomtags[] = {
    GA_RELRIGHT,	LZOOMRELRIGHT,
    GA_TOP,		0,
    GA_SYSGTYPE,	WZOOM,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	ldepthtags[] = {
    GA_RELRIGHT,	LDEPTHRELRIGHT,
    GA_TOP,		0,
    GA_SYSGTYPE,	WUPFRONT,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	lsdepthtags[] = {
    GA_RELRIGHT,	LDEPTHRELRIGHT,
    GA_TOP,		0,
    GA_SYSGTYPE,	SUPFRONT,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};


struct TagItem	lsizetags[] = {
    GA_RELRIGHT,	-12,
    GA_RELBOTTOM,	-10,
    GA_SYSGTYPE,	SIZING,
    GA_RELVERIFY,	TRUE,
    TAG_END,
};

struct TagItem	*gtaglists[][2] = {
    {depthtags,ldepthtags},
    {zoomtags,lzoomtags},	/* also old "downback"		*/
    {sizetags,lsizetags},
    {closetags,lclosetags},
    {wdragtags,wdragtags},		/* wdrag		*/
    {sdepthtags,lsdepthtags},		/* sdepth/upfront	*/
    {NULL,NULL},			/* sdownback (obsolete)	*/
    {sdragtags,sdragtags},		/* sdrag		*/
};

struct Gadget	*
createNewSysGadget( s, res, gindex, gimme )
struct XScreen	*s;
USHORT res, gindex;
BOOL gimme;
{
    struct Image	*getScreenSysImage();
    struct Gadget	*copySysGadget();
    struct Gadget 	*g = NULL;
    struct TagItem	*gtags;
    void		*NewObject();
    ClassID		classid;
    extern UBYTE	*GadgetClassName;
    extern UBYTE	*ButtonGClassName;
    struct Image	*gimage;

    /* Most sys-gadgets are buttongclass */
    classid = ButtonGClassName;
    if ( gindex == DRAGGADGET || gindex == SDRAGGADGET )
    {
	/* except for the drag gadgets */
	classid = GadgetClassName;
    }

    DSG( printf("createNSG s %lx, res %lx, gindex %lx, gimme %lx class %s\n",
	s, res, gindex, gimme, classid ) );

    /* if I have tags defined, then I want to use a boopsi gadget */
    if ( gtags = gtaglists[ gindex ][ res ] )
    {
	DSG( printf("get new gadget for index %lx and tags %lx\n",
		gindex, gtags ) );
	DSG( printf(" image at %lx\n", s->SysImages[ gindex ] ) );

	/* select image from XScreen array of images */
	if ( ( gimage = getScreenSysImage( s, gindex ) ) != ((struct Image *)-1) )
	{
	    g = (struct Gadget *) NewObject( NULL, classid, 
		    /* use "create-as-needed" sys images	*/
		    GA_IMAGE, gimage,
		    GA_GZZGADGET, gimme,
		    GA_SYSGADGET, TRUE,
		    GA_GadgetHelp, TRUE,
		    TAG_MORE, gtags );

	    DSG( printf("result: %lx\n", g ) );
	}
    }

    return( g );	/* returns NULL if no tags specified */
}

/* 
 * returns per-screen system image, or creates it if:
 *	- it doesn't exist, and
 *	- it's not a draggadget or otherwise invisible
 *
 * Returns -1 if there's an error.
 */
static struct Image	*
getScreenSysImage( s, gindex )
struct Screen	*s;
{
    int imsize;
    extern UBYTE	*SysIClassName;
    struct Image	**sysimg;

    /* invisible gadgets	*/
    if ( gindex == DRAGGADGET || gindex == SDRAGGADGET ) return ( NULL );

    /* The sysimsg pointer saves ROM space */
    sysimg = &XSC(s)->SysImages[ gindex ];

    /* create an image if not already created	*/
    if ( *sysimg == NULL )
    {
	imsize = SYSISIZE_LOWRES;
	if ( IsHires( s ) )
	{
	    imsize = SYSISIZE_MEDRES;
	}


	if ( !( *sysimg = NewObject( NULL, SysIClassName,
	    /* sysiclass uses this BarHeight, if relevant.
	     * SYSIA_Size default comes from DrawInfo
	     */
	    IA_HEIGHT,	s->BarHeight + 1,
	    SYSIA_DrawInfo,	&( XSC(s)->SDrawInfo ),
	    SYSIA_Which,	 gindex, 
	    TAG_END ) ) )
	{
	    /* Peter 25-Jan-91: Return code for error */
	    return( (struct Image *) -1 );
	}
    }
    return ( *sysimg );
}

#define VARIABLE_THICKNESS 0

/* Peter 23-Mar-92
 * Martin gave me a brand-new embossedBoxTrim() that handles
 * variable thickness and GadTools style bevelled corners.
 *
 * I took out thickness support except hthick can be 1 or 2.
 * My version also infers hthick = 2 if join-type is JOINS_ANGLED.
 * Otherwise, hthick is 1.
 *
 * behavior at "joins" in upper-right and lower-left corners
 * is controlled by value of 'meeting_type':
 * JOINS_UPPER_LEFT_WINS (0) - The color used to render the upper and left
 *	sides of the box is used in the joins.
 * JOINS_LOWER_RIGHT_WINS (1) - The color used to render the lower and right
 *	sides of the box is used in the joins.
 * JOINS_NONE (2) - The joins are not rendered.
 * JOINS_ANGLED (3) - The joins are angled, in the style that GadTools uses.
 */
#if !VARIABLE_THICKNESS

VOID
embossedBoxTrim( rp, b, ulpen, lrpen, meeting_type )
struct RastPort *rp;
struct IBox *b;
ULONG ulpen;
ULONG lrpen;
ULONG meeting_type;
{
    LONG left, top, bottom, right;
    LONG scum1, scum2, scum3, scum4;	/*dave is scum */
    ULONG hthick = 1;
    LONG i;

    if ((b->Width > 0) && (b->Height > 0))
    {
	scum1 = 1;
	scum2 = 1;
	scum3 = 1;
	scum4 = 1;

	switch ( meeting_type )
	{
	    case JOINS_UPPER_LEFT_WINS:
		scum1 = scum2 = 0;
		break;

	    case JOINS_LOWER_RIGHT_WINS:
		scum3 = scum4 = 0;
		break;

	    case JOINS_ANGLED:
		scum2 = scum3 = 0;
		hthick = 2;
		break;

#if 0
	    case_JOINS_NONE:
#endif
	    default:
		break;
	}

	left   = b->Left;
	top    = b->Top;
	right  = boxRight(b);
	bottom = boxBottom(b);

	SetABPenDrMd(rp, ulpen, 0, JAM1);
	BNDRYOFF( rp );
	SetAfPt( rp, NULL, 0 );

	/* top edge */
	RectFill(rp,left,top,right-scum1,top);

	/* left edge */
	for (i = 0; i < hthick; i++)
	{
	    RectFill(rp,left+i,top+1,left+i,bottom-scum2);
	    scum2 = 1;
	}

	SetAPen(rp,lrpen);

	/* right edge */
	for (i = 0; i < hthick; i++)
	{
	    RectFill(rp,right-i,top+scum3,right-i,bottom-1);
	    scum3 = 1;
	}

	/* bottom edge */
	RectFill(rp,left+scum4,bottom,right,bottom);
    }
}

#else /* variable thickness version */

VOID
embossedBoxTrim( rp, b, hthick, vthick, ulpen, lrpen, meeting_type )
struct RastPort *rp;
struct IBox *b;
ULONG hthick;
ULONG vthick;
ULONG ulpen;
ULONG lrpen;
ULONG meeting_type;
{
    LONG left, top, bottom, right;
    LONG cnt;
    LONG scum1, scum2, scum3, scum4;	/*dave is scum */
    LONG i;

    if ((b->Width > 0) && (b->Height > 0))
    {
	scum1 = 1;
	scum2 = 1;
	scum3 = 1;
	scum4 = 1;

	switch ( meeting_type )
	{
	    case JOINS_UPPER_LEFT_WINS:
		scum1 = scum2 = 0;
		break;

	    case JOINS_LOWER_RIGHT_WINS:
		scum3 = scum4 = 0;
		break;

	    case JOINS_ANGLED:
		scum2 = scum3 = 0;
		break;

#if 0
	    case_JOINS_NONE:
#endif
	    default:
		break;
	}

	left   = b->Left;
	top    = b->Top;
	right  = boxRight(b);
	bottom = boxBottom(b);

	SetAPen(rp,ulpen);

	/* top edge */
	cnt = scum1;
	for (i = 0; i < vthick; i++)
	{
	    RectFill(rp,left,top+i,right-cnt,top+i);

	    if (cnt < hthick)
		cnt++;
	}

	/* left edge */
	cnt = scum2;
	for (i = 0; i < hthick; i++)
	{
	    RectFill(rp,left+i,top+vthick,left+i,bottom-cnt);

	    if (cnt < vthick)
		cnt++;
	}

	SetAPen(rp,lrpen);

	/* right edge */
	cnt = scum3;
	for (i = 0; i < hthick; i++)
	{
	    RectFill(rp,right-i,top+cnt,right-i,bottom-vthick);

	    if (cnt < vthick)
		cnt++;
	}

	/* bottom edge */
	cnt = scum4;
	for (i = 0; i < vthick; i++)
	{
	    RectFill(rp,left+cnt,bottom-i,right,bottom-i);

	    if (cnt < hthick)
		cnt++;
	}
    }
}
#endif



/* return width/height border requirements for
 * sizing gadget used in windows of specified screen
 *
 * these dimensions are compatible: width is old sizing gadget + 2,
 * height is old sizeg height + 1.
 */
getSizeGDims( sc, dims )
struct Screen	*sc;
UWORD		dims[2];
{
    if ( IsHires( sc ) )
    {
	dims[0] = 18;	/* width */
	dims[1] = 10;
    }
    else
    {
	dims[0] = 13;	/* width */
	dims[1] = 11;
    }
}
