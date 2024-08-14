/* demotextb.c -- demonstration of 		:ts=8
 * Basic Object Oriented Programming System for Intuition
 *
 * Demonstrates use of a text button object (from class "textbclass").
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

#define D(x)	;

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct  Library         *UtilityBase;

Object	*NewObject();

ULONG	myiflags = GADGETDOWN | GADGETUP | CLOSEWINDOW;

/* objects to be created	*/
void		*initTextBClass();
void		*TextBClass = NULL;
struct Gadget	*textbutton = NULL;

void		*initFrame1Class();
void		*Frame1Class = NULL;
struct Image	*frameimage = NULL;

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct DrawInfo	*drinfo;

    struct Window 	*OpenWindowTags();
    struct Window	*w;

    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }

    D( printf("about to openwindow\n") );
    w = OpenWindowTags( NULL, 
		WA_Title,	"boopsi Test Window",
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

    /* create a frame image which can be shared by contents
     * of different sizes
     */
    Frame1Class = initFrame1Class();
    frameimage =  (struct Image *) NewObject( Frame1Class, NULL, TAG_END );

    /* create a text button gadget using
     * a "simple ascii string" as a label
     */
    TextBClass = initTextBClass();
    textbutton =  (struct Gadget *) NewObject( TextBClass, NULL, 
		GA_LEFT,	30,
		GA_TOP,		20,
		GA_TEXT,	"My Gadget",
		GA_IMAGE,	frameimage,
		GA_ID,		0xf00d,
		GA_DRAWINFO,	drinfo,		/* textbuttons need this now */
    		TAG_END );

    D(printf("objects initialized\n"));

    /* make it a little bigger, but want to see centered text */
    SetGadgetAttrs( textbutton, NULL, NULL, 
		    	GA_WIDTH, textbutton->Width + 20,
			GA_DRAWINFO,	drinfo,		/* since no window */
			TAG_END );

    AddGList( w, textbutton, -1, -1, NULL );
    RefreshGList( textbutton, w, NULL, -1 );

    D( printf("gadgets added and refreshed \n") );

    goHandleWindow( w );

    RemoveGList( w, textbutton, -1 );
    FreeScreenDrawInfo( w->WScreen, drinfo );
    CloseWindow( w );

    D( printf("dispose\n") );
    DisposeObject( textbutton );
    DisposeObject( frameimage);
    freeTextBClass( TextBClass );
    freeFrame1Class( Frame1Class );

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
    struct IntuiMessage *imsg;
    struct Gadget	*g;

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

	    case GADGETDOWN:
		D( printf("gadget down, %lx\n", imsg->IAddress ) );
		break;

	    case GADGETUP:
		g = (struct Gadget *) imsg->IAddress;
		D( printf("gadget up, %lx, id %lx\n",
			imsg->IAddress, g->GadgetID ) );
		break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }
}
