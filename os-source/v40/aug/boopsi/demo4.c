/* demo4.c -- demonstration of 		:ts=8
 * Basic Object Oriented Programming System for Intuition
 *
 * This demo shows the construction of a composite (group)
 * gadget, collecting all four gadgets used in the
 * other demos.
 *
 * The application can use this gadget as a single entity
 * with either the GADGETUP or IDCMPUPDATE messages.
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

ULONG	myiflags =  CLOSEWINDOW | GADGETUP | IDCMPUPDATE;
struct Window	*w = NULL;

Object		*mymodel = NULL; /* the repository of the "Current Value" */
void	*MyModClass = NULL;
void	*initMyModClass();


struct Gadget	*groupg = NULL;

/* pictures for arrows	*/
struct Image	*upimage = NULL;
struct Image	*downimage = NULL;

#define PWIDTH		(120)	/* width of horizontal propslider	*/
#define SWIDTH		(50)

#define PROPRANGE	(20)
#define INITVAL		(0)	/* initial value of string and slider	*/

enum gadgetids {
    gUp = 1,
    gDown,
    gSlider,
    gString,
    gGroup,
};

/* a macro for the "adjacent" position to the right of a gadget,
 * but safe, if the reference gadget is NULL
 */
#define RIGHTBY( g )	( ((g)==NULL)? 20: ((g)->LeftEdge + (g)->Width ) )

/****************************************************************/
/*  mapping tag lists						*/
/****************************************************************/

/* for IDCMPUPDATE	*/
struct TagItem	model2me[] = {
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


struct TagItem	proptags[] = {
    {GA_WIDTH,		PWIDTH},	/* height to be specified	*/

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

#define MYWINDOWTITLE	"boopsi Demo 4"

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct Window 	*OpenWindowTags();	/* in varargs.c for now */

    struct Gadget	*g;
    struct DrawInfo	*drinfo;

    WORD		nextleft;

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

    D( printf("get group 'master' gadget\n"));

    /* get a group gadget which will manage our four basic gadgets */
    groupg = (struct Gadget *) NewObject( NULL, GROUPGCLASS,
				GA_LEFT,	w->BorderLeft,
				GA_TOP,		w->BorderTop,
				GA_ID,		gGroup,
    				TAG_END );
    if ( ! groupg ) cleanup( "can't get group gadget" );
    D( printf("group gadget at %lx\n", groupg ) );

    /* get "model" object which is the repository of our "current
     * value" and is the hub of object interconnection.
     * This thing also is used to free icclass objects,
     * so we'd better make sure it got allocated.
     */
    MyModClass = initMyModClass();	/* private class	*/
    D( printf("get model object, class at %lx\n", MyModClass ) );

    /* We connect the model directly to ourselves, the
     * group gadget isn't in the loop.
     */
    mymodel = (Object *) NewObject( MyModClass, NULL, 
			ICA_TARGET,	ICTARGET_IDCMP,
			ICA_MAP,	model2me,
			TAG_END );
    D( printf("model at %lx\n", mymodel ) );

    if ( ! mymodel ) cleanup( "couldn't get model object" );

    /* get the string gadget	*/
    g  = (struct Gadget *) NewObject( NULL, "strgclass",
    	GA_LEFT,	0,
	ICA_TARGET,	mymodel,
	TAG_MORE,	stringtags,
	TAG_END );
    D( printf( "string gadget at %lx\n", g ) );

    /* when I 'addmember' this to the group, 
     * the group willoffset its position, so
     * if I want to use it, it has to be now
     */
    nextleft = RIGHTBY( g );

    /* add it to the group ...	*/
    DoMethod( groupg, OM_ADDMEMBER, g );

    /* The string gadget now talks to the model.
     * Add an interconnection from the model to
     * the string gadget.
     */
    ic = NewObject( NULL, ICCLASS,
    		ICA_TARGET,	g,
		ICA_MAP,	model2string,
		TAG_END );
    DoMethod( mymodel, OM_ADDMEMBER, ic );

    /* get the prop gadget, immediately to the right of the string */
    g = (struct Gadget *) NewObject( NULL, "propgclass",
	GA_LEFT,	nextleft,
	GA_HEIGHT,	downimage? downimage->Height: 20,
	ICA_TARGET,	mymodel,
	TAG_MORE,	proptags,
	TAG_END );
    D( printf( "prop gadget returned: %lx\n", g ) );
    nextleft = RIGHTBY( g );

    /* add it to the group ...	*/
    DoMethod( groupg, OM_ADDMEMBER, g );

    /* ... and get an ic for the prop gadget	*/
    ic = NewObject( NULL, ICCLASS,
    		ICA_TARGET,	g,
		ICA_MAP,	model2slider,
		TAG_END );
    DoMethod( mymodel, OM_ADDMEMBER, ic );

    /* down button is immediately to the right ...	*/
    g = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	downimage,
	GA_LEFT,	nextleft,
	GA_ID,		gDown,
	/* interconnections ...	*/
	ICA_TARGET,	mymodel,
	ICA_MAP,	&downarrow2model[0],
	TAG_END );
    D( printf("downgadget at %lx\n", g ));
    nextleft = RIGHTBY( g );

    /* ... and add it to the group gadget	*/
    DoMethod( groupg, OM_ADDMEMBER, g );

    /* the buttons don't need to hear from the model, 
     * hence, no ic's for them.
     */

    /* up arrow button	*/
    g = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	upimage,
	GA_LEFT,	nextleft,
	GA_ID,		gUp,
	/* interconnections ...	*/
	ICA_MAP,	&uparrow2model[0],
	ICA_TARGET,	mymodel,
	TAG_END );
    D( printf("upgadget at %lx\n", g ));

    /* ... and add it to the group gadget	*/
    DoMethod( groupg, OM_ADDMEMBER, g );

    D(printf("all objects initialized\n"));

    /* As a variation on demo3, we'll initialize the
     * values of everything before we add it to the window,
     * and we can use the more vanilla SetAttrs() function.
     *
     * I can't call SetGadgetAttrs() for 'groupg' and have
     * it propagate, since the group gadget doesn't know
     * anything about the model, in this example.
     */
     SetAttrs( mymodel, 
		MYMODA_RANGE, PROPRANGE,
		MYMODA_CURRVAL, PROPRANGE/2,
		TAG_END );

    /* check out the positioning of a group gadget */
    SetGadgetAttrs( groupg, w, NULL,
    		GA_LEFT, 4 * w->BorderLeft,
		GA_TOP,	 3 * w->BorderTop,
		TAG_END );

    /* And here's the nice part:
     * you add just the ONE gadget
     * to the window.
     */
    AddGList( w, groupg, -1, 1, NULL );
    RefreshGList( groupg, w, NULL, -1 );

    D( printf("gadgets added and refreshed \n") );

    goHandleWindow( w );

    RemoveGList( w, groupg, 1 );	/* just one	*/

    FreeScreenDrawInfo( w->WScreen, drinfo );
    cleanup( "all done" );
}

cleanup( str )
char	*str;
{
    if (str) printf("%s\n", str);

    D( printf("dispose objects\n") );
    DisposeObject( mymodel );

    /* and more fun part: you toss this away, and all associated
     * gadget components go with it.
     */
    DisposeObject( groupg );

    DisposeObject( upimage);
    DisposeObject( downimage);
    D( printf("have disposed.\n") );

    if ( MyModClass ) freeMyModClass( MyModClass );

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

		GetAttr( MYMODA_CURRVAL, mymodel, &currval );
		D( printf("Current value now %ld\n", currval ) );

		break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }
}

