/* demo3.c -- demonstration of 		:ts=8
 * Basic Object Oriented Programming System for Intuition
 *
 * This demo shows gadgets and images created using pre-defined
 * boopsi classes, interconnected using boopsi interconnections
 * and a "model".
 *
 * The improvement over demo 2 is that the application only
 * receives messages it is specifically interested in.  All
 * inter-gadget traffic is handled (synchronously) among 
 * the objects involved.
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

/* from varargs.c -- interface to NewObjectA()	*/
Object	*NewObject();

ULONG	myiflags =  CLOSEWINDOW | IDCMPUPDATE;

Object		*mymodel = NULL; /* the repository of the "Current Value" */
void	*MyModClass = NULL;
void	*initMyModClass();


struct Gadget	*propg = NULL;
struct Gadget	*stringg = NULL;
struct Gadget	*uparrowg = NULL;
struct Gadget	*downarrowg = NULL;
struct Gadget	*mygadgets = NULL;		/* linked list	*/

/* pictures for arrows	*/
struct Image	*upimage = NULL;
struct Image	*downimage = NULL;

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


/****************************************************************/
/*  mapping tag lists						*/
/****************************************************************/

/* for IDCMPUPDATE	*/
struct TagItem	model2me[] = {
    /* {MYMODA_CURRVAL, MYMODA_CURRVAL }, */
    {MYMODA_CURRVAL, ICSPECIAL_CODE },
    /* put (16 bits of) currval into IntuiMessage.Code	*/
    { TAG_END, }
};

struct TagItem	slider2model[] = {
    {PGA_TOP, MYMODA_CURRVAL},
    {TAG_END, }
};

struct TagItem	model2slider[] = {
    {MYMODA_CURRVAL, PGA_TOP},
    {MYMODA_RANGE, PGA_TOTAL },
    {TAG_END, }
};

struct TagItem	string2model[] = {
    {STRINGA_LongVal, MYMODA_CURRVAL},
    {TAG_END, }
};

struct TagItem	model2string[] = {
    {MYMODA_CURRVAL, STRINGA_LongVal},
    {TAG_END, }
};

struct TagItem	uparrow2model[] = {
    {GA_ID, MYMODA_INCRSTROBE},
    {TAG_END, }
};

struct TagItem	downarrow2model[] = {
    {GA_ID, MYMODA_DECRSTROBE},
    {TAG_END, }
};

/****************************************************************/
/* tag lists for creating objects				*/
/****************************************************************/

struct TagItem modeltags[] = {
    { ICA_TARGET,	ICTARGET_IDCMP},	/* talk to me	*/
    { ICA_MAP,		(ULONG) &model2me[0]},
    {TAG_END ,}
};

struct TagItem	proptags[] = {
    {GA_TOP,		GTOP},
    {GA_WIDTH,		PWIDTH},	/* height to be specified	*/
    {GA_ID,		gSlider},

    /* {ICA_TARGET,	XXX }, * will be model object	*/
    {ICA_MAP,		(ULONG) &slider2model[0]},

    {PGA_FREEDOM,	FREEHORIZ},
    {PGA_VISIBLE,	1},		/* want an integer value slider	*/

#if 0	/* the whole set of gadgets will be initialized together	*/
    {PGA_TOP,		INITVAL},	/* "top" in the scroller sense	*/
    {PGA_TOTAL,		PROPRANGE},
#endif

    {TAG_END ,}
};

struct TagItem	stringtags[] = {
    {GA_ID,		gString},
    {GA_TOP,		GTOP},
    {GA_WIDTH,		SWIDTH},
    {GA_HEIGHT,		12},		/* fix this	*/

    /* {ICA_TARGET,	XXXX },		will be model object	*/
    {ICA_MAP,		(ULONG) &string2model[0]},

    {STRINGA_MaxChars,	10},
    {STRINGA_LongVal,	INITVAL},	/* make it an integer gadget */
    {STRINGA_Justification,
    			STRINGRIGHT},
    {TAG_END, }
};

struct TagItem	uparrowtags[] = {
    {GA_TOP,		GTOP},
    /* {ICA_TARGET,	ICTARGET_IDCMP}, will be model object	*/
    {ICA_MAP,		(ULONG) &uparrow2model[0]},
    {TAG_END, }
};

struct TagItem	downarrowtags[] = {
    {GA_TOP,		GTOP},
    /* {ICA_TARGET,	ICTARGET_IDCMP}, will be model object	*/
    {ICA_MAP,		(ULONG) &downarrow2model[0]},
    {TAG_END, }
};

#define MYWINDOWTITLE	"boopsi Demo 3"

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct Window 	*OpenWindowTags();	/* in varargs.c for now */

    struct Gadget	*tmpgad;
    struct Window	*w;
    struct DrawInfo	*drinfo;

    Object		*ic;

    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }

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

    /* get images for the up and down arrows, sensitive
     * to depth and pen specs for current screen (we'll be
     * adding resolution/size selection later).
     */
    upimage = (struct Image *) NewObject( NULL, "sysiclass",
	SYSIA_Size,	0,		/* normal "medium-res" for now */
	SYSIA_DrawInfo, drinfo,
	SYSIA_Which,	UPIMAGE,
    	TAG_END );

    downimage = (struct Image *) NewObject( NULL, "sysiclass",
	SYSIA_Size,	0,		/* normal "medium-res" for now */
	SYSIA_Which,	DOWNIMAGE,
	SYSIA_DrawInfo, drinfo,
    	TAG_END );


    /* get "model" object which is the repository of our "current
     * value" and is the hub of object interconnection.
     * This thing also is used to free icclass objects,
     * so we'd better make sure it got allocated.
     */
    MyModClass = initMyModClass();	/* private class	*/
    D( printf("get model object, class at %lx\n", MyModClass ) );

    mymodel = (Object *) NewObjectA( MyModClass, NULL, modeltags );
    D( printf("model at %lx\n", mymodel ) );

    if ( ! mymodel ) cleanup( "couldn't get model object" );

    /* make gadgets, link into list (easier starting with Beta 4) */
    tmpgad = (struct Gadget *) &mygadgets;

    D( printf("get gadgets\n"));

    downarrowg = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	downimage,
	GA_TOP,		GTOP,
	GA_LEFT,	ARROWLEFT,
	GA_ID,		gDown,
	GA_PREVIOUS,	tmpgad,
	/* interconnections ...	*/
	ICA_TARGET,	mymodel,
	ICA_MAP,	&downarrow2model[0],
	TAG_END );
    D( printf("downgadget at %lx\n", downarrowg ));

    /* get up/down arrow button gadgets	*/
    uparrowg = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	upimage,
	GA_TOP,		GTOP,
	GA_LEFT,	ARROWLEFT + (downarrowg? downarrowg->Width: 0),
	GA_ID,		gUp,
	GA_PREVIOUS,	tmpgad,
	/* interconnections ...	*/
	ICA_MAP,	&uparrow2model[0],
	ICA_TARGET,	mymodel,
	TAG_END );
    D( printf("upgadget at %lx\n", uparrowg ));

    propg = (struct Gadget *) NewObject( NULL, "propgclass",
	GA_LEFT,	ARROWLEFT - PWIDTH,
	GA_HEIGHT,	downarrowg? downarrowg->Height: 20,
	GA_PREVIOUS,	tmpgad,

	ICA_TARGET,	mymodel,
	TAG_MORE,	proptags,
	TAG_END );
    D( printf( "prop gadget returned: %lx\n", propg ) );

    stringg  = (struct Gadget *) NewObject( NULL, "strgclass",
    	GA_LEFT,	propg? (propg->LeftEdge - SWIDTH): 20,
	GA_PREVIOUS,	tmpgad,

	ICA_TARGET,	mymodel,
	TAG_MORE,	stringtags,
	TAG_END );
    D( printf( "string gadget at %lx\n", stringg ) );

    /*
     * We now have all the gadgets talking to the model,
     * but we need to create some little IC nodes for
     * the model to update the string and propotional gadgets
     */
    ic = NewObject( NULL, ICCLASS,
    		ICA_TARGET,	stringg,
		ICA_MAP,	model2string,
		TAG_END );
    DoMethod( mymodel, OM_ADDMEMBER, ic );

    ic = NewObject( NULL, ICCLASS,
    		ICA_TARGET,	propg,
		ICA_MAP,	model2slider,
		TAG_END );
    DoMethod( mymodel, OM_ADDMEMBER, ic );

    D(printf("objects initialized\n"));

    AddGList( w, mygadgets, -1, -1, NULL );
    RefreshGList( mygadgets, w, NULL, -1 );

    D( printf("gadgets added and refreshed \n") );

    /* although we're changing the attributes of
     * the model, if we want the gadgets attached 
     * to it to be able to refresh, we have to 
     * use SetGadgetAttr()s rather than SetAttrs()
     */
    SetGadgetAttrs( mymodel, w, NULL,
    			MYMODA_RANGE, PROPRANGE,
			MYMODA_CURRVAL, PROPRANGE/2,
			TAG_END );

    D( printf("have set range and initval\n" ) );

    goHandleWindow( w );

    RemoveGList( w, mygadgets, -1 );
    FreeScreenDrawInfo( w->WScreen, drinfo );
    CloseWindow( w );
    cleanup( "all done" );
}

cleanup( str )
char	*str;
{
    struct Gadget	*tmpgad;

    if (str) printf("%s\n", str);

    D( printf("dispose objects\n") );
    DisposeObject( mymodel );

    while ( mygadgets )
    {
	tmpgad = mygadgets->NextGadget;
	DisposeObject( mygadgets );
	mygadgets = tmpgad;
    }

    DisposeObject( upimage);
    DisposeObject( downimage);
    D( printf("have disposed.\n") );

    if ( MyModClass ) freeMyModClass( MyModClass );

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

	    case IDCMPUPDATE:
		tags = (struct TagItem *) imsg->IAddress;
	    	D( printf("IDCMPUPDATE, quals %lx code (decimal) %ld\n",
			imsg->Qualifier, imsg->Code ));

		GetAttr( MYMODA_CURRVAL, mymodel, &currval );
		D( printf("Current value now %ld\n", currval ) );

		break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }
}

