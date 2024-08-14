/*** ezreq.c *****************************************************************
 *
 *  EasyRequester 
 *
 *  $Id: ezreq.c,v 38.15 93/01/12 16:17:38 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include <exec/memory.h>
#include <dos/dosextens.h>
#include "ezreq.h"
#include "classusr.h"
#include "imageclass.h"
#include "gadgetclass.h"

#include "ezreq_protos.h"

/*---------------------------------------------------------------------------*/

static struct Window * commonBSR(struct Window * rwin,
                                 struct Screen * refscreen,
                                 UBYTE * eztitle,
                                 struct IntuiText * itext,
                                 struct Gadget * glist,
                                 Object * gadgetframe,
                                 ULONG iflags,
                                 struct Remember ** remkey,
                                 BOOL layout_body);

static int reqTitleLength(struct RastPort * rp,
                          UBYTE * title);

static int addGadImage(struct Gadget * gad,
                       struct Image * im);

static int disposeGadgetList(struct Gadget * g);

static int disposeImageList(struct Image * im);

static struct Window * reqWindow(struct Window * rwin,
                                 struct Screen * screen,
                                 UBYTE * title,
                                 struct Gadget * glist,
                                 WORD reqWidth,
                                 WORD reqHeight,
                                 ULONG iflags);

static int processFail(BOOL fail);

/*---------------------------------------------------------------------------*/

#define D(x)	;
#define DEZ(x)	;
#define DSR(x)	;
#define DFSR(x)	;
#define DAR(x)	;
#define DMEM(x)	;

struct Window	*BuildEasyRequestArgs();
/*** intuition.library/EasyRequestArgs ***/

/* Intuitionface.asm calls this through stackSwap */

easyRequestArgs( rwin, ez, idcmp_ptr, args )
struct Window	*rwin;
struct EasyStruct *ez;
ULONG		*idcmp_ptr;
UWORD		*args;
{
    struct Window	*window;
    int			retval = 0;
    struct Window	*buildEasyRequestArgs();

    DMEM( printf("EasyRequestArgs in\n"));

    window = buildEasyRequestArgs(rwin, ez, idcmp_ptr? *idcmp_ptr: 0, args );

    /* window could be 'TRUE' or 'FALSE', as well as valid pointer */
    while ( (retval = SysReqHandler( window, idcmp_ptr, TRUE )) == -2 )
    {
	DEZ( printf("EasyR try again for input\n") );
	/* loop	*/;
    }

    DMEM( printf("EasyR free\n") );
    FreeSysRequest( window );
    DMEM( printf("EasyR returning %lx\n", retval ) );

    return ( retval );
}

/*** intuition.library/AutoRequest ***/


/* in intuitionface.asm, this gets called via stackSwap() */

BOOL
autoRequest( rwin, bodyt, goodt, badt, goodif, badif, width, height )
struct Window		*rwin;
struct IntuiText	*bodyt;
struct IntuiText	*goodt;
struct IntuiText	*badt;
ULONG			goodif;
ULONG			badif;
WORD			width;
WORD			height;
{
    int retval;
    struct Window	*window;
    ULONG		idcmp;
    APTR		buildSysRequest();

    DAR( printf("AutoRequest: rwin %lx, task %lx\n", rwin, FindTask( 0 ) ));
    DMEM( printf("AutoRequest in\n"));

    idcmp = goodif | badif;

    window = (struct Window *)buildSysRequest( rwin, 
	    bodyt, goodt, badt, idcmp, width, height);

    DAR( printf("window open\n") );

    if ( window == NULL || (int) window == 1)
    {
	return ( (int) window );
    }

    while ( (retval = SysReqHandler( window, &idcmp, TRUE )) == -2 )
    {
	/* loop	*/;
    }

    DAR( printf("AutoRequest satisfied, retval %ld\n", retval ) );

    /* do "goodflags/badflags" test	*/
    if ( retval == -1 )
    {
	retval =  ( idcmp & goodif )? 1: 0;
	DAR( printf("AutoRequest good/bad flags, retval %ld\n", retval ) );
    }

    FreeSysRequest( window );
    DAR( printf(" AR done, returning %lx\n", retval ) );

    DMEM( printf("AR done\n"));
    return ( retval );
}
/*** intuition.library/SysReqHandler ***/

/*
 * might return a gadgetid = 0, 1, ...
 * or 'no message' (-2), or 'user idcmp (-1)'
 *
 * 'no message' also can mean 'was rawkey, but wrong code'
 */
SysReqHandler( w, idcmp_ptr, wait_first )
struct Window	*w;
ULONG		*idcmp_ptr;
{
    struct IntuitionBase	*IBase = fetchIBase();
    UBYTE	code;

    int	windowval = (ULONG) w;
    struct IntuiMessage	*imsg;
    int			retval = -2;

    DSR( printf(" sRH enter\n") );

    if ( windowval == 0 || windowval == 1 )
    {
	DSR( printf("sRH says screw it, windowval %lx\n", windowval ) );
	return ( windowval );
    }

    if ( wait_first )
    {
	WaitPort( w->UserPort );
    }

    while ( retval == -2 &&
	(imsg = (struct IntuiMessage *) GetMsg( w->UserPort ) ) )
    {
	if ( imsg->Class == GADGETUP )
	{
	    retval = ((struct Gadget *) imsg->IAddress)->GadgetID;
	}
	else if ( imsg->Class == VANILLAKEY )
	{
	DSR( printf("got a vanillakey %lc (0x%lx)\n", imsg->Code, imsg->Code));

	    if ( imsg->Qualifier & IEQUALIFIER_LCOMMAND )
	    {
		/* new IControl Prefs	*/
		code = ToUpper( imsg->Code );

		if ( code == ToUpper( IBase->ReqFalseCode ) )
		    retval = 0;
		else if ( code == ToUpper( IBase->ReqTrueCode ) )
		    retval = 1;
	    }
	}
	else
	{
	    retval = -1;	/* tell him IDCMP */
	    if ( idcmp_ptr ) *idcmp_ptr = imsg->Class;
	}

	ReplyMsg( imsg );
    }
    return ( retval );
}
/*** intuition.library/BuildEasyRequestArgs ***/


/* In intuitionface.asm, this is called via stackSwap() */

struct Window	*
buildEasyRequestArgs( rwin, ez, iflags, args )
struct Window	*rwin;
struct EasyStruct *ez;
ULONG		iflags;
UWORD		*args;
{
    struct Remember	*remember = NULL;

    struct ITList	*formatITList();
    struct Window	*commonBSR();
    struct Screen	*LockPubScreen();
    struct Gadget	*createITGadget();

    struct ITList	*bodyitl = NULL;
    struct ITList	*gaditl = NULL;

    struct Window	*window = NULL;

    struct Screen	*screen = NULL;
    struct Gadget	*gadgets = NULL;
    struct Gadget	*g;
    int			id;
    struct IntuiText	*it;
    struct IntuiText	*succ_it;	/* successor	*/

    UWORD		*arglist = args;	/* varargs */

    Object		*frame = NULL;
    Object		*NewObject();

    extern UBYTE	*FrameIClassName;

    D( printf("BEasyR enter\n") );

    /* 8-Mar-91:  If I'm a process, initialize pr_Result2 to NULL */
    processFail( FALSE );

    /* pass arglists through RawDoFmt, and end up with
     * linked lists of IntuiText: one per line for body text,
     * one per gadget (for now, anyway) for gadgets
     */
    bodyitl = formatITList( ez->es_TextFormat,arglist,&arglist,&remember,'\n');
    gaditl = formatITList(ez->es_GadgetFormat,arglist,&arglist,&remember,'|');
    D( printf("BEasyR got text lists %lx %lx\n", bodyitl, gaditl) );
    if ( !( bodyitl && gaditl ) ) goto BAILOUT;

    /* get screen and gadgets early	*/
    if ( ! ( screen = rwin? rwin->WScreen: LockPubScreen( NULL ) ) )
	goto BAILOUT;
    D( printf("BEasyR: rwin at %lx, screen at %lx\n", rwin, screen ) );

    /* (screen && !rwin) means I'm on a public screen	*/

    /* get frame image object shared by gadgets	*/
    if ( ! ( frame = NewObject( NULL, FrameIClassName,
	TAG_END ) ) )
	goto BAILOUT;

    DFSR( printf("allocated frame object at %lx\n", frame ));

    /* get the gadgets, one per IntuiText for now	*/
    id = 1;
    g = (struct Gadget *) &gadgets;	/* 'g' is 'last gadget' */
    for ( it = gaditl->itl_IText; it; it = succ_it )
    {
	succ_it = it->NextText;		/* stash successor	*/
	it->NextText = NULL;		/* break link 		*/

	/*** 
	it->FrontPen = GTEXTCOLOR;
	it->DrawMode = 0;
	it->TopEdge = GADVMARGIN;
	it->LeftEdge = GADHMARGIN ;
	***/

	if (! (g = createITGadget( screen, it, &remember, g, id++, frame ) ))
	    goto BAILOUT;
    }
    g->GadgetID = 0;	/* last one is always zero	*/

    D( printf("BEasyR calling cBSR\n") );
    window =  commonBSR( rwin, screen, ez->es_Title,
			bodyitl->itl_IText, gadgets, frame,
			iflags, &remember, TRUE );	/* do layout	*/

    /*** Don't allocate anything else on 'remember' after commonBSR() ***/

    if ( window == NULL )
    {
	D( printf("BEasyR: no window back from BSR\n") );
	goto BAILOUT;
    }

    goto DONE;

BAILOUT:
    D( printf("BEasyR freeing: bail out\n") );

    /* free boopsi gadgets here	*/
    disposeGadgetList( gadgets );

    FreeRemember( &remember, TRUE );
    DisposeObject( frame );

    /* 8-Mar-91:  If I'm a process, fail with ERROR_NO_FREE_STORE */
    processFail( TRUE );

DONE:
    D( printf("BEasyR success: %lx\n", window ) );
    if ( screen && !rwin )
    {
	D( printf("BEasyR: Unlocking pubscreen %lx\n", screen ) );
	UnlockPubScreen( NULL, screen );
    }
    return ( window );
}
/*** intuition.library/BuildSysRequest ***/


/* In intuitionface.asm, this function gets called via stackSwap()	 */

APTR
buildSysRequest( rwin, bodyt, retryt, cancelt, iflags, width, height )
struct Window		*rwin;
struct IntuiText	*bodyt;
struct IntuiText	*retryt;
struct IntuiText	*cancelt;
ULONG			iflags;
{
    struct Remember	*remember = NULL;
    struct Screen	*screen;
    struct Gadget	*gadgets = NULL;
    struct Gadget	*g;
    struct Window	*window = NULL;
    struct Window	*commonBSR();

    Object		*frame = NULL;
    Object		*NewObject();
    extern UBYTE	*FrameIClassName;

    D(printf( "In rBSR\n" ) );
    if ( ! cancelt ) return ( NULL );	/* quick check	*/

    if ( ! ( screen = rwin? rwin->WScreen: LockPubScreen( NULL ) ) )
    {
	DMEM( printf("no lock pub, bailout\n"));
	goto BAILOUT;
    }
    /* (screen && !rwin) means I'm on a public screen	*/

    /* get frame image object shared by gadgets	*/
    if ( ! ( frame = NewObject( NULL, FrameIClassName,
	TAG_END ) ) )
	goto BAILOUT;

    DFSR( printf("allocated frame object at %lx\n", frame ));

    /* create one or two gadgets from provided text	*/
    if (!(g=createITGadget( screen, cancelt, &remember, &gadgets, 0, frame)))
    {
	DMEM( printf("no cancel gadget\n"));
	goto BAILOUT;
    }

    /* try to get (optional) retry gadget, if text was provided	*/
    if ( retryt )
    {
	if (!createITGadget( screen, retryt, &remember, &gadgets, 1, frame )) 
	{
	    DMEM( printf("createITG retryg bailout\n"));
	    goto BAILOUT;
	}
    }

    window =  commonBSR( rwin, screen, NULL,
			bodyt, gadgets, frame,
			iflags, &remember, FALSE );	/* don't layout	*/
    /*** Don't allocate anything else on 'remember' after commonBSR() ***/

    if ( window == NULL )
    {
	DMEM( printf("BEasyR: no window back from BSR\n") );
	goto BAILOUT;
    }

    goto DONE;

BAILOUT:
    DMEM( printf("BSR freeing: bail out\n") );

    /* free boopsi gadgets here	*/
    disposeGadgetList( gadgets );

    FreeRemember( &remember, TRUE );
    DisposeObject( frame );

DONE:
    if ( screen && !rwin )
    {
	UnlockPubScreen( NULL, screen );
    }
    D( printf( "Exiting rBSR with window = %lx\n", window ) );
    return ( window );
}

#define	ezreqPattern	CPattern
#define ezreqPatSize	(1)

/* resolution precalculations in ticks	*/
#define VSPACETICKS	(4 * MOUSESCALEY)	/* vertical margins	*/
#define HSPACETICKS	(4 * MOUSESCALEX)	/* horiz margins	*/
#define HGADSPACETICKS	(12 * MOUSESCALEX)	/* between gadgets	*/
#define HTEXTMARGINTICKS (20 * MOUSESCALEX)	/* text in box		*/

/*
 * Common BuildSysRequest/BuildEasyRequest
 * will NOT FreeRemember if it aborts, that's up to caller
 * if I return NULL or (s Window *) 1.
 * Don't allocate anything else on 'remember' after commonBSR()!
 */
static struct Window	*
commonBSR( rwin, refscreen, eztitle, itext, glist, gadgetframe,
	iflags, remkey, layout_body )
struct Window	*rwin;
struct Screen	*refscreen;
UBYTE		*eztitle;
struct IntuiText	*itext;
struct Gadget	*glist;
Object		*gadgetframe;
ULONG		iflags;
struct Remember	**remkey;
BOOL		layout_body;
{
    struct Window	*reqWindow();
    struct DrawInfo	*GetScreenDrawInfo();
    extern UWORD	ezreqPattern[];

    struct Window	*retwindow = NULL;
    struct SysReqTrack	*srt;

    int gadcount;
    struct DrawInfo	*drinfo;
    struct Image	*itextimage;
    struct Image	*textframe;
    struct IBox		framebox;
    struct IBox		gadgetbox;
    WORD		vspace, hspace;
    WORD		hgadspace;
    STRPTR		title;
    struct IBox		reqBox;
    struct Gadget	*image_gad;

    extern UBYTE	*FillRectClassName;
    extern UBYTE	*FrameIClassName;
    extern UBYTE	*ITextIClassName;
    extern UBYTE	*GadgetClassName;

    DMEM( printf("commonBSR enter glist %lx, itext %lx\n", glist, itext) );

    reqBox.Left = refscreen->WBorLeft;
    reqBox.Top =  refscreen->WBorTop + refscreen->Font->ta_YSize + 1;

    /*** allocate Image-carrying gadget and SysReqTrack	***/
    if ( !( image_gad=(struct Gadget *)AllocRemember( remkey,
	sizeof *srt + sizeof *image_gad, MEMF_CLEAR ) ) )
    {
	DMEM( printf("cBSR no requester\n") );
	goto BAILOUT;
    }
    image_gad->Flags = ( GFLG_GADGIMAGE | GFLG_GADGHNONE );

    srt = (struct SysReqTrack *) &image_gad[1];
    srt->srt_GadgetFrame = gadgetframe;

    title = (STRPTR) fetchIBase()->SysReqTitle;
    if (rwin)
    {
	title = rwin->Title;
    }
    if (eztitle)
    {
	title = eztitle;
    }

    /*** layout intuitext, if needed, and get their extent	***/
    ITextLayout( &refscreen->RastPort, itext, &framebox, layout_body,
	AUTOTOPEDGE, AUTOLEFTEDGE );

    drinfo = GetScreenDrawInfo( refscreen );

    /* calc a few resolution sensitive quantities	*/
    vspace = VSPACETICKS / drinfo->dri_Resolution.Y;
    hspace = HSPACETICKS / drinfo->dri_Resolution.X;
    hgadspace = HGADSPACETICKS / drinfo->dri_Resolution.X;

    /*** create an IText image object, and a frame for text	***/
    itextimage = (struct Image *) NewObject( NULL, ITextIClassName,
			IA_DATA, 	itext,
			IA_FGPEN,	drinfo->dri_Pens[ TEXTPEN ],
			IA_WIDTH,	framebox.Width + 2 * framebox.Left,
			IA_HEIGHT,	framebox.Height + 2 * framebox.Top,
			TAG_END );
    addGadImage( image_gad, itextimage );

    textframe = (struct Image *) NewObject( NULL, FrameIClassName,
			IA_RECESSED,	TRUE,
			TAG_END );
    addGadImage( image_gad, textframe );

    if ( ! ( itextimage && textframe ) ) 
    {
	DMEM( printf("no text image/frame\n"));
	goto BAILOUT;
    }

    /*** layout scheme revolves around the 'textframe' width ***/
    /* get nominal frame dimensions	*/
    SendMessage(  textframe, IM_FRAMEBOX,
	    &itextimage->LeftEdge, &framebox, drinfo, 0 );

    /* get nominal gadget dimensions	*/
    /* ZZZ: if we want to make gadgets vertical, do
     * it using this function.
     */
    gadcount = glistExtentBox( glist, &gadgetbox, hgadspace );

    /* textframe width adds a little space beside text,
     * and is as large as gadgets and requester window title
     */
    framebox.Width = imax( gadgetbox.Width,
	framebox.Width + 2 * ( HTEXTMARGINTICKS / drinfo->dri_Resolution.X ) );

    /* Make framebox as wide as needed for the title, but don't let the
     * title make it wider than the screen.
     * (24 and 18 are the hires/lores widths of the depth gadget)
     */
    framebox.Width = imax( framebox.Width,
	imin( ( (IsHires( refscreen )? 24: 18) +
	    reqTitleLength( &refscreen->RastPort, title ) ),
	    refscreen->Width - ( refscreen->WBorLeft + refscreen->WBorRight +
	    2*hspace )
	    ) );

    /* don't change gadgetbox.Width yet: needed in spreadG */

    framebox.Left = gadgetbox.Left = hspace + reqBox.Left;
    framebox.Top = vspace + reqBox.Top;
    gadgetbox.Top = boxBottom( &framebox ) + vspace;

    /* Peter 24-Jan-91: To ensure that the buttons fit on-screen,
     * we enforce the following relationship:
     */
     gadgetbox.Top = imin(gadgetbox.Top, refscreen->Height -
	(gadgetbox.Height + vspace ) );

    /*** position real gadgets	***/
    spreadLayoutGadgets( glist, gadcount, &gadgetbox,
	framebox.Width, hgadspace);

    /*** position frame and intuitext object	***/
    SetAttrs( textframe, 
		IA_LEFT, framebox.Left,
		IA_TOP,  framebox.Top,
		IA_WIDTH, framebox.Width,
		IA_HEIGHT, framebox.Height,
		TAG_END );

    /* get centered position of contents, relative to frame dimensions */
    SendMessage(  textframe, IM_FRAMEBOX,
	    &itextimage->LeftEdge, &framebox, drinfo, FRAMEF_SPECIFY );

    /* now, framebox has in left/top the offsets from the contents
     * origin (0,0) to the frame position.  We push the contents back
     * so that the frame position of framebox.Left/Top holds 
     */
    SetAttrs( itextimage, 
		IA_LEFT, textframe->LeftEdge - framebox.Left,
		IA_TOP, textframe->TopEdge - framebox.Top,
		TAG_END );

    /*** calculate requester box	***/
    reqBox.Width = boxRight( &textframe->LeftEdge ) - reqBox.Left + hspace + 1;
    reqBox.Height = boxBottom( &gadgetbox ) - reqBox.Top + vspace + 1;

    /* toss in the dithered rectfill background	*/
    /* re-using 'textframe' here 		*/
    textframe = (struct Image *)NewObject( NULL, FillRectClassName,
			IA_FGPEN,	drinfo->dri_Pens[ SHINEPEN ],
			IA_LEFT,	reqBox.Left,
			IA_TOP,		reqBox.Top,
			IA_WIDTH,	reqBox.Width,
			IA_HEIGHT,	reqBox.Height,
			IA_APATTERN,	ezreqPattern,
			IA_APATSIZE,	ezreqPatSize,
			IA_MODE,	JAM2,
    
			TAG_END );
    /* It's OK if 'fillrect' object isn't successfully created.
     * addGadImage() safely does nothing if the image is NULL.
     */
    addGadImage( image_gad, textframe );

    /* Put our image gadget at the end of the gadget list */
    {
	struct Gadget *runner;
	for ( runner = glist; runner->NextGadget; runner = runner->NextGadget )
		;
	runner->NextGadget = image_gad;
    }

    /*** open window for requester	***/
    /* ZZZ: debugging test: bail out and see how graceful */
    /*** retwindow = NULL; goto BAILOUT;	***/

#if 	0	/* bailout test	*/
    if ( retwindow = NULL )	/* always fail	*/
#else
    if ( retwindow = reqWindow( rwin, refscreen, title, glist,
	reqBox.Width, reqBox.Height, iflags) )
#endif
    {

	/* OK, got window, return everything back to caller */
	srt->srt_Remember = *remkey;
	srt->srt_ReqGadgets = glist;
	srt->srt_ImageGad = image_gad;
	retwindow->UserData = (BYTE *)srt;	/* track resource */
	FreeScreenDrawInfo( refscreen, drinfo );
	DMEM( printf("return OK\n"));
	return ( retwindow );
    }
    DMEM( printf("cBSR no reqWindow\n" ) );

BAILOUT:
    DMEM( printf("commonBSR BAILOUT!!!\n"));
    /* abort: should be going to an alert here	*/
    FreeScreenDrawInfo( refscreen, drinfo );

    /* free all memory I allocated which is NOT covered
     * by FreeRemember, which my caller will be freeing.
     */
    if ( image_gad )
    {
	DMEM( printf("disposeImageList now\n"));
	disposeImageList( image_gad->GadgetRender );
    }

    /* 8-Mar-91:  If I'm a process, fail with ERROR_NO_FREE_STORE */
    processFail( TRUE );

    return ( NULL );
}

/*
 * looks like we need to tear off trailing spaces
 */
static
reqTitleLength( rp, title )
struct RastPort	*rp;
UBYTE		*title;
{
    int	len;
    UBYTE	*tail;

    if ( !title )
    {
	return( 0 );
    }

    len = strlen( title );

    tail = title + (len - 1);	/* point to last valid char	*/
    while ( tail > title && *tail == ' ' )
    {
	tail--;
    }

    return ( TextLength( rp, title, tail - title + 1 ) );
}

/* adds image to front of image list,
 * which means its visuals are IN BACK of other elements
 * already in list (since it's drawn first).
 */
static
addGadImage( gad, im )
struct Gadget	*gad;
struct Image	*im;
{
    if ( im )
    {
	im->NextImage = gad->GadgetRender;
	gad->GadgetRender = im;
    }
}


static
disposeGadgetList( g )
struct Gadget	*g;
{
    struct Gadget	*succ_g;
    for ( ; g; g = succ_g )
    {
	succ_g =  g->NextGadget;

	if ( (g->GadgetType & GTYPEMASK) == CUSTOMGADGET )
	{
	    DisposeObject( g );
	}
    }
}

static
disposeImageList( im )
struct Image	*im;
{
    struct Image	*succ_im;

    for ( ; im; im = succ_im )
    {
	DFSR( printf("im %lx, succ %lx\n", im, im->NextImage ) );
	succ_im =  im->NextImage;
	DisposeObject( im );
    }
}

static struct Window	*
reqWindow( rwin, screen, title, glist, reqWidth, reqHeight, iflags )
struct Window	*rwin;
struct Screen	*screen;
UBYTE		*title;
struct Gadget 	*glist;
WORD		reqWidth;
WORD		reqHeight;
ULONG		iflags;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window	*w;
    struct Screen	*reqscreen;
    struct Window	*openWindowOnlyTags();
    ULONG		screentag;

    DMEM( printf("enter reqWindow\n"));

    /* By default, we're going onto a public screen */
    screentag = WA_PubScreen;

    if (rwin)	/* override Title and Type */
    {
	screentag = WA_CustomScreen;
    }

    /* Peter 2-Dec-90: Limit testing on window size will be done
     * through WA_AutoAdjust, which is on by default since we're
     * not supplying a NewWindow structure.
     *
     * WA_AutoAdjust also means that we no longer have to supply
     * values for WA_Left and WA_Top to ensure that I appear on the
     * visible part of a scrolled oversize screen.
     */

    if ( w = openWindowOnlyTags(
	WA_Flags, ( WFLG_ACTIVATE | WFLG_SIMPLE_REFRESH | WFLG_DRAGBAR |
	            WFLG_DEPTHGADGET | WFLG_NOCAREREFRESH | WFLG_RMBTRAP ),
	WA_Left, imax( 0, -screen->LeftEdge ),
	WA_Top, imax( 0, -screen->TopEdge ),
	WA_InnerWidth, reqWidth,
	WA_InnerHeight, reqHeight,

	WA_IDCMP, iflags | REQIFLAGS,
	screentag, screen,
	WA_Title, title,
	WA_Gadgets, glist,
	TAG_DONE ) )
    {
	DMEM( printf("rW: window open %lx\n", w ) );

	/* EasyRequests are automatically in the same HelpGroup
	 * as the reference window.
	 */
	if ( rwin )
	{
	    XWINDOW(w)->HelpGroup = XWINDOW(rwin)->HelpGroup;
	}

	/**** POP screen front ***/
	if ( (reqscreen = w->WScreen) != IBase->FirstScreen )
	{
	    /* IScreenDepth() will clear poppedscreen,
	     * so do it synchronously
	     */
	    ScreenDepth( reqscreen, SDEPTH_TOFRONT, NULL );
	    SETFLAG( IBase->Flags, POPPEDSCREEN );
	}
    }

OUT:

    DMEM( printf("rW to return %lx\n", w ));
    return ( w );
}

/*** intuition.library/FreeSysRequest ***/

FreeSysRequest(window)
struct Window *window;
{
    struct Screen	*reqscreen;
    struct SysReqTrack	*srt;

    DFSR( printf("FSR\n") );

    if ( window == NULL || (ULONG) window == 1 ) return;

    /* catch a pointer to my gadgets before I end the requester */
    srt = (struct SysReqTrack *)window->UserData;

    reqscreen = window->WScreen;

    /* close the requestwindow */
    CloseWindow(window);

    /*** POP screen back ***/
    if ( TESTFLAG( fetchIBase()->Flags, POPPEDSCREEN ) )
    {
	DAR( printf("pop screen back\n") );
	/* Peter 7-Jan-91:  We must depth-arrange the screen
	 * synchronously since the caller might call CloseScreen()
	 * too soon after, if I used sendISMNoQuick instead.
	 */
	ScreenDepth( reqscreen, SDEPTH_TOBACK, NULL );

	/* POPPEDSCREEN cleared inside IDepthScreen */
    }

    disposeGadgetList( srt->srt_ReqGadgets );
    disposeImageList( srt->srt_ImageGad->GadgetRender );

    /* free the things I've tracked; do FreeRemember() LAST, since
     * it free's the SysReqTrack
     */
    DFSR( printf("dispose gadget frame %lx\n", srt->srt_GadgetFrame) );

    DisposeObject( srt->srt_GadgetFrame );

    DFSR( printf("FreeRemember %lx\n", &srt->srt_Remember, TRUE) );

    FreeRemember( &srt->srt_Remember, TRUE );
}

/* Peter 8-Mar-91: DOS needs to be able to sense the difference between
 * ...Request() returning a result of zero because the user selected
 * "Cancel", and a result of zero owing to out-of-memory.  The problem
 * being addressed is that if EasyRequest() fails for lack of memory
 * on a "Software Failure" requester, DOS will believe that the user
 * selected "Reboot".
 */

static
processFail( fail )
BOOL fail;
{
    struct Task *FindTask();

    struct Process *myself = (struct Process *)FindTask( NULL );

    if ( myself->pr_Task.tc_Node.ln_Type == NT_PROCESS )
    {
	myself->pr_Result2 = ( fail ) ? ERROR_NO_FREE_STORE : 0;
    }
}
