/* demo5.c -- demonstration of 		:ts=8
 * Basic Object Oriented Programming System for Intuition
 *
 * This demo shows the simple use of a composite gadget
 * class; as simple as creating one boopsi gadget and
 * using it, with either IDCMPUPDATE or GADGETUP.
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
#include "mymodel.h"

#define D(x)	x

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct  Library         *UtilityBase;

Object	*NewObject();

ULONG	myiflags =  CLOSEWINDOW | GADGETUP | IDCMPUPDATE;

void	*MyGroupClass = NULL;
void	*initMyGroupClass();

struct Gadget	*mygadget = NULL;
struct Window	*w = NULL;

#define PROPRANGE	(20)
#define INITVAL		(0)	/* initial value of string and slider	*/

/****************************************************************/
/*  mapping tag list						*/
/****************************************************************/

/* for IDCMPUPDATE	*/
struct TagItem	gadget2me[] = {
    {MYMODA_CURRVAL, ICSPECIAL_CODE },
    /* put (16 bits of) currval into IntuiMessage.Code	*/
    { TAG_END, }
};

#define MYWINDOWTITLE	"boopsi Demo 5"

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct Window 	*OpenWindowTags();	/* in varargs.c for now */

    struct DrawInfo	*drinfo;

    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }

    /* initialize my private class	*/
    if ( ! ( MyGroupClass = initMyGroupClass() ) )
    {
	cleanup( "can't initialize gadget group" );
    }

    D( printf(" my group class is at %lx\n", MyGroupClass ) );

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
    D( printf("window at %lx\n", w ) );

    if ( w == NULL) cleanup( "couldn't open my window.\n");
    drinfo = GetScreenDrawInfo( w->WScreen );

    /* Go get the big guy.
     * Note that I pass the DrawInfo to this *gadget*,
     * using GA_DRAWINFO (not SYSIA_DrawInfo).  Sorry.
     */
    mygadget = (struct Gadget *) NewObject( MyGroupClass, NULL,
				GA_LEFT,	4 * w->BorderLeft,
				GA_TOP,		3 * w->BorderTop,
				GA_ID,		99,
				GA_DRAWINFO,	drinfo,

				ICA_TARGET,	ICTARGET_IDCMP,
				ICA_MAP,	gadget2me,

    				TAG_END );
    if ( ! mygadget ) cleanup( "can't get composite gadget" );
    D( printf("group gadget at %lx\n", mygadget ) );

    /* 
     * Now that the model and gadget are packaged together,
     * I just talk to the gadget.  I pass NULL window
     * since the gadget is not yet attached.
     */
     SetGadgetAttrs( mygadget, NULL, NULL,
		MYMODA_RANGE, PROPRANGE,
		MYMODA_CURRVAL, PROPRANGE/2,
		TAG_END );

    /* And here's the nice part:
     * you add just the ONE gadget
     * to the window.
     */
    AddGList( w, mygadget, -1, 1, NULL );
    RefreshGList( mygadget, w, NULL, -1 );

    D( printf("gadgets added and refreshed \n") );

    goHandleWindow( w );

    RemoveGList( w, mygadget, 1 );	/* just one	*/

    FreeScreenDrawInfo( w->WScreen, drinfo );
    cleanup( "all done" );
}

cleanup( str )
char	*str;
{
    if (str) printf("%s\n", str);

    /* the only thing I have to clean up is the gadget. */
    DisposeObject( mygadget );

    if ( MyGroupClass )
    {
	int freesuccess;

	freesuccess = freeMyGroupClass( MyGroupClass );
	D( if ( ! freesuccess ) printf("!!! MyGroupClass refused to free!\n");)
    }

    if ( w ) CloseWindow( w );
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (GfxBase) CloseLibrary(GfxBase);
    if (UtilityBase) CloseLibrary(UtilityBase);

    exit (0);
}

goHandleWindow( w )
struct Window	*w;
{
    struct TagItem	*FindTagItem();

    struct IntuiMessage *imsg;
    struct TagItem	*tags;
    ULONG		currval;
    /* struct TagItem	*ti; */

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

	   case GADGETUP:
	   	D( printf("GADGETUP GadgetID: %lx\n",
			((struct Gadget *) imsg->IAddress)->GadgetID ) );
		break;

	    case IDCMPUPDATE:
		tags = (struct TagItem *) imsg->IAddress;
	    	D( printf("IDCMPUPDATE, quals %lx code (decimal) %ld\n",
			imsg->Qualifier, imsg->Code ));

		GetAttr( MYMODA_CURRVAL, mygadget, &currval );
		D( printf("Current value now %ld\n", currval ) );

		break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }
}

