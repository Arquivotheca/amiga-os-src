/* demo2.c -- demonstration of 		:ts=8
 * Basic Object Oriented Programming System for Intuition
 *
 * This demo shows gadgets and images created using pre-defined
 * boopsi classes, but not connected together or bound into a group.
 * An improvement over demo1.c is that the buttons, slider, and string
 * notify us using the new IDCMPUPDATE change mechanism for "real-time"
 * updates of values.
 */

/*
Copyright (c) 1989 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"
#include <intuition/icclass.h>

#define D(x)	;

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct  Library         *UtilityBase;

/* from varargs.c -- interface to NewObjectA()	*/
Object	*NewObject();

ULONG	myiflags =  CLOSEWINDOW | IDCMPUPDATE;

struct Gadget	*propg;
struct Gadget	*stringg;
struct Gadget	*uparrowg;
struct Gadget	*downarrowg;

/* pictures for arrows	*/
struct Image	*upimage;
struct Image	*downimage;

/* some static layout and setup constants, for now	*/
#define GTOP		(44)
#define ARROWLEFT	(180)
#define PWIDTH		(120)	/* width of horizontal propslider	*/
#define SWIDTH		(50)
#define PROPRANGE	(20)
#define INITVAL		(0)	/* initial value of string and slider	*/

enum gadgetids {
    gUp = 1,
    gDown,
    gSlider,
    gString,
};

#define MYTAG_CURRVAL	(TAG_USER + 999)

/* mapping tag lists for IDCMPUPDATE	*/
struct TagItem	slider2me[] = {
    {GA_ID, GA_ID},
    {PGA_TOP, MYTAG_CURRVAL},
    {TAG_END, }
};

struct TagItem	arrows2me[] = {
    {GA_ID, GA_ID},
    {TAG_END, }
};

struct TagItem	string2me[] = {
    {GA_ID, GA_ID},
    {STRINGA_LongVal, MYTAG_CURRVAL},
    {TAG_END, }
};

struct TagItem	proptags[] = {
    {GA_TOP,		GTOP},
    {GA_WIDTH,		PWIDTH},	/* height to be specified	*/
    {GA_ID,		gSlider},

    {ICA_TARGET,	ICTARGET_IDCMP},
    {ICA_MAP,		(ULONG) &slider2me[0]},

    {PGA_FREEDOM,	FREEHORIZ},
    {PGA_TOP,		INITVAL},	/* "top" in the scroller sense	*/
    {PGA_VISIBLE,	1},		/* want an integer value slider	*/
    {PGA_TOTAL,		PROPRANGE},

    {TAG_END ,}
};

struct TagItem	stringtags[] = {
    {GA_ID,		gString},
    {GA_TOP,		GTOP},
    {GA_WIDTH,		SWIDTH},
    {GA_HEIGHT,		12},		/* fix this	*/

    {ICA_TARGET,	ICTARGET_IDCMP},
    {ICA_MAP,		(ULONG) &string2me[0]},

    {STRINGA_MaxChars,	10},
    {STRINGA_LongVal,	INITVAL},	/* make it an integer gadget */
    {STRINGA_Justification,
    			STRINGRIGHT},
    {TAG_END, }
};

struct TagItem	arrowtags[] = {
    {GA_TOP,		GTOP},

    {ICA_TARGET,	ICTARGET_IDCMP},
    {ICA_MAP,		(ULONG) &arrows2me[0]},
    {TAG_END, }
};

#define MYWINDOWTITLE	"boopsi Demo 2"

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct Window 	*OpenWindowTags();	/* in varargs.c for now */

    struct Gadget	*mygadgets = NULL;
    struct Gadget	*tmpgad;
    struct Window	*w;
    struct DrawInfo	*drinfo;

    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }

    D( printf("about to openwindow\n") );
    w = OpenWindowTags( NULL, 
		WA_Title,	MYWINDOWTITLE,
		WA_SimpleRefresh, TRUE,
		WA_NoCareRefresh, TRUE,
		WA_DepthGadget,	TRUE,
		WA_DragBar,	TRUE,
		WA_Left,	300,
		WA_Top,		50,
		WA_Width,	280,
		WA_Height,	120,
		WA_IDCMP,	myiflags,
		WA_CloseGadget,	TRUE,
    		TAG_END );
    D( printf("window at %lx\n ", w ) );

    if ( w == NULL) cleanup( "couldn't open my window.\n");
    drinfo = GetScreenDrawInfo( w->WScreen );

    /* get images for the up and down arrows, sensitive
     * to depth and pen specs for current screen (we'll be
     * adding resolution/size selection later).
     */
    upimage = (struct Image *) NewObject( NULL, "sysiclass",
	SYSIA_Size,	0,		/* normal "medium-res" for now */
	SYSIA_Which,	UPIMAGE,
	SYSIA_DrawInfo,	drinfo,
    	TAG_END );

    downimage = (struct Image *) NewObject( NULL, "sysiclass",
	SYSIA_Size,	0,		/* normal "medium-res" for now */
	SYSIA_Which,	DOWNIMAGE,
	SYSIA_DrawInfo,	drinfo,
    	TAG_END );

    /* make gadgets, link into list (easier starting with Beta 4) */
    tmpgad = (struct Gadget *) &mygadgets;

    downarrowg = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	downimage,
	GA_LEFT,	ARROWLEFT,
	GA_ID,		gDown,
	GA_PREVIOUS,	tmpgad,
	TAG_MORE,	arrowtags,

	TAG_END );
    D( printf("downgadget at %lx\n", downarrowg ));

    /* get up/down arrow button gadgets	*/
    uparrowg = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	upimage,
	GA_LEFT,	ARROWLEFT + (downarrowg? downarrowg->Width: 0),
	GA_ID,		gUp,
	GA_PREVIOUS,	tmpgad,
	TAG_MORE,	arrowtags,
	TAG_END );
    D( printf("upgadget at %lx\n", uparrowg ));


    propg = (struct Gadget *) NewObject( NULL, "propgclass",
	GA_LEFT,	ARROWLEFT - PWIDTH,
	GA_HEIGHT,	downarrowg? downarrowg->Height: 20,
	GA_PREVIOUS,	tmpgad,
	TAG_MORE,	proptags,
	TAG_END );
    D( printf( "prop gadget returned: %lx\n", propg ) );

    stringg  = (struct Gadget *) NewObject( NULL, "strgclass",
    	GA_LEFT,	propg? (propg->LeftEdge - SWIDTH): 20,
	GA_PREVIOUS,	tmpgad,
	TAG_MORE,	stringtags,
	TAG_END );
    D( printf( "string at %lx\n", stringg ) );

    D(printf("objects initialized\n"));

    AddGList( w, mygadgets, -1, -1, NULL );
    RefreshGList( mygadgets, w, NULL, -1 );

    D( printf("gadgets added and refreshed \n") );

    goHandleWindow( w );

    RemoveGList( w, mygadgets, -1 );
    FreeScreenDrawInfo( w->WScreen, drinfo );
    CloseWindow( w );

    D( printf("dispose\n") );
    while ( mygadgets )
    {
	tmpgad = mygadgets->NextGadget;
	DisposeObject( mygadgets );
	mygadgets = tmpgad;
    }

    DisposeObject( upimage);
    DisposeObject( downimage);
    D( printf("have disposed.\n") );

    cleanup( "all done" );
}

cleanup( str )
char	*str;
{
    if (str) printf("%s\n", str);

    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (GfxBase) CloseLibrary(GfxBase);
    if (UtilityBase) CloseLibrary(UtilityBase);

    exit (0);
}

/* this variable holds the integer "current value" of the
 * whole little system of gadgets
 */
LONG		currval = 0;

goHandleWindow( w )
struct Window	*w;
{
    struct TagItem	*FindTagItem();

    struct IntuiMessage *imsg;
    struct TagItem	*tags;
    /* struct TagItem	*ti; */
    ULONG		gid;

    for(;;)
    {
	WaitPort( w->UserPort );
	while ( imsg = (struct IntuiMessage *) GetMsg( w->UserPort ) )
	{
	    switch ( imsg->Class )
	    {
	    case CLOSEWINDOW:
	    	ReplyMsg( (struct Message *) imsg );
		return;

	    case IDCMPUPDATE:
		tags = (struct TagItem *) imsg->IAddress;
	    	D( printf("idcmpupdate, tags at %lx quals %lx\n",
			tags, imsg->Qualifier ));
		D( printf("gadget id %lx\n", GetTagData( GA_ID, 0, tags )));

		switch (  gid = GetTagData( GA_ID, 0, tags ) )
		{
		case gString:
		case gSlider:
		    currval =  GetTagData( MYTAG_CURRVAL, 0, tags );
		    manualUpdate( w, gid );
		    break;

		case gUp:
		    currval++;
		    manualUpdate( w, 0 );
		    break;
		case gDown:
		    currval--;
		    manualUpdate( w, 0 );
		    break;
		    /* cases where button GA_ID's are negative are 
		     * repeat codes when the pointer is OFF
		     * the button, and are ignored
		     */
		}
		break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }
}

manualUpdate( w, activeid )
struct Window	*w;
{
    /* put currval in legal bounds	*/
    currval = max( 0, min( PROPRANGE-1, currval ) );

    /* tell interested parties, but not active slider		*/
    if ( activeid != gSlider )
    {
	SetGadgetAttrs( propg, w, NULL, 
		PGA_TOP, currval,
		TAG_END );
    }

    /* always OK to update string, and it needs limited currval
     * to boot.
     */
    SetGadgetAttrs( stringg, w, NULL, 
	STRINGA_LongVal, currval,
	TAG_END );
}
