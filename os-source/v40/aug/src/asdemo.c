/*
** asdemo.c:  Attached screens demo
**
** Compile as lc -L
**
*/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

void printf(STRPTR,...);
void exit(int);

#ifdef DEBUGGING
#define DP(x)	kprintf x
#else
#define DP(x)	;
#endif

/*------------------------------------------------------------------------*/

void main(void);
void open_up();
void close_down();
void enter();
void leave(int, STRPTR);
BOOL HandleGadgetEvent(struct Window *, struct Gadget *, UWORD);
struct Gadget *CreateAllGadgets(struct Gadget **, void *, UWORD);

void initNodes();
void orderNodes( struct List *list );

struct ScreenNode *findFreeNode();
struct ScreenNode *findScreenNode( struct Screen *s );
struct ScreenNode *findScreenNodeOrder( WORD order );
void ScreenUp( int behind, struct ScreenNode *parent );
void ScreenDown( struct ScreenNode *node );
void AllScreensDown();

/*------------------------------------------------------------------------*/

#ifndef SA_Attach
#define SA_Attach	(SA_Dummy + 0x001D)
#endif

/*------------------------------------------------------------------------*/

/* Gadget defines of our choosing, to be used as GadgetID's: */

#define GAD_LISTVIEW	1
#define GAD_TOFRONT	2
#define GAD_TOBACK	3
#define GAD_NEWFRONT	4
#define GAD_NEWBACK	5
#define GAD_CHILDFRONT	6
#define GAD_CHILDBACK	7
#define GAD_CLOSE	8
#define GAD_REFRESH	9

/*------------------------------------------------------------------------*/

#define FRONT	0
#define BACK	1

/*------------------------------------------------------------------------*/

struct TextAttr Topaz80 =
{
    "topaz.font",	/* Name */
    8,			/* YSize */
    0,			/* Style */
    0,			/* Flags */
};

extern struct Library *SysBase;
struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *GadToolsBase = NULL;
struct Screen *mysc = NULL;
struct Gadget *glist = NULL;
struct Window *mywin = NULL;
void *vi = NULL;

struct List screens;

BOOL terminated = FALSE;
struct Gadget *lvgad, *textgad;
UWORD topborder;
BOOL rethink = FALSE;
ULONG ScreenNumber = 0;

char *ErrorString[] =
{
    "",
    "No screen selected",
    "This screen doesn't belong to me",
    "Child screens can't have children",
    "Can't close -- children still open",
};

#define NO_SCREEN_SELECTED	1
#define SCREEN_NOT_OURS		2
#define ALREADY_CHILD		3
#define SCREEN_HAS_CHILDREN	4

/*------------------------------------------------------------------------*/

UWORD pens[] =
{
    ~0,
};

/*------------------------------------------------------------------------*/

#define NUMNODES 50
#define TITLESIZE 50

struct ScreenNode
{
    struct Node sn_Node;
    BOOL sn_Ours;		/* Did we create it? */
    ULONG sn_Order;		/* In-list ordering */

    struct Screen *sn_Screen;	/* Pointer to screen */
    struct ScreenNode *sn_Parent;	/* Node of parent screen */
    WORD sn_ScreenNumber;	/* Serial number of this screen */
    WORD sn_ChildNumber;	/* Serial number of children of this parent */
    WORD sn_Children;		/* Number of children open */
    char sn_Title[TITLESIZE];	/* Screen's title */
};

struct ScreenNode nodes[NUMNODES];
struct ScreenNode *selnode = NULL;

/*------------------------------------------------------------------------*/

void main(void)
{
    struct IntuiMessage *imsg;
    struct Gadget *gad;
    ULONG imsgClass;
    UWORD imsgCode;

    enter();
    mysc = LockPubScreen( NULL );
    open_up();
    UnlockPubScreen( NULL, mysc );

    while (!terminated)
    {
	Wait (1 << mywin->UserPort->mp_SigBit);
	/* GT_GetIMsg() returns a cooked-up IntuiMessage with
	 * more friendly information for complex gadget classes.  Use
	 * it wherever you get IntuiMessages:
	 */
	while ((!terminated) && (imsg = GT_GetIMsg(mywin->UserPort)))
	{
	    imsgClass = imsg->Class;
	    imsgCode = imsg->Code;
	    /* Presuming a gadget, of course, but no harm... */
	    gad = (struct Gadget *)imsg->IAddress;
	    /* Use the toolkit message-replying function here... */
	    GT_ReplyIMsg(imsg);
	    switch (imsgClass)
	    {
		case GADGETUP:
		    rethink = HandleGadgetEvent(mywin, gad, imsgCode);
		    break;

		case GADGETDOWN:
		    rethink = HandleGadgetEvent(mywin, gad, imsgCode);
		    break;

		case CLOSEWINDOW:
		    terminated = TRUE;
		    break;

		case REFRESHWINDOW:
		    /* You must use GT_BeginRefresh() where you would
		     * normally have your first BeginRefresh()
		     */
		    GT_BeginRefresh(mywin);
		    GT_EndRefresh(mywin, TRUE);
		    break;
	    }
	    if ( mywin->WScreen != ( mysc = IntuitionBase->FirstScreen ) )
	    {
		close_down();
		open_up();
	    }
	    else if ( rethink )
	    {
		GT_SetGadgetAttrs( lvgad, mywin, NULL,
		    GTLV_Labels, ~0,
		    TAG_DONE );
		orderNodes( &screens );
		GT_SetGadgetAttrs( lvgad, mywin, NULL,
		    GTLV_Labels, &screens,
		    TAG_DONE );
		rethink = FALSE;
	    }
	}
    }
    leave(0, NULL);
}

/*------------------------------------------------------------------------*/

void open_up()
{
    orderNodes( &screens );

    if (!(vi = GetVisualInfo(mysc,
	TAG_DONE)))
	leave(20, "GetVisualInfo() failed");

    /* Here is how we can figure out ahead of time how tall the
     * window's title bar will be:
     */
    topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);

    if (!CreateAllGadgets(&glist, vi, topborder))
    {
	leave(20, "CreateAllGadgets() failed");
    }

    if (!(mywin = OpenWindowTags(NULL,
	WA_Width, 320,
	WA_Height, 181,

	WA_Activate, TRUE,
	WA_DragBar, TRUE,
	WA_DepthGadget, TRUE,
	WA_CloseGadget, TRUE,
	WA_SimpleRefresh, TRUE,

	WA_IDCMP, INTUITICKS | CLOSEWINDOW | REFRESHWINDOW | \
	    LISTVIEWIDCMP| BUTTONIDCMP,

	WA_MinWidth, 50,
	WA_MinHeight, 50,
	WA_MaxWidth, ~0,
	WA_MaxHeight, ~0,
	WA_Title, "Attached Screen Demo",

	/* Gadgets go here, or in NewWindow.FirstGadget */
	WA_Gadgets, glist,
	WA_CustomScreen, mysc,

	TAG_DONE)))
	leave(20, "OpenWindow() failed");

    /* After window is open, we must call this GadTools refresh
     * function.
     */
    GT_RefreshWindow(mywin, NULL);
}

/*------------------------------------------------------------------------*/

/*/ close_down()
 *
 * Function to remove the window
 */

void close_down()
{
    if (mywin)
    {
	CloseWindow(mywin);
	mywin = NULL;
    }

    /* None of these two calls mind a NULL parameter, so it's not
     * necessary to check for non-NULL before calling.  If we do that,
     * we must be certain that the OpenLibrary() of GadTools succeeded,
     * or else we would be jumping into outer space:
     */
    if (GadToolsBase)
    {
	FreeVisualInfo(vi);
	vi = NULL;
	FreeGadgets(glist);
	glist = NULL;
    }
}

/*------------------------------------------------------------------------*/

void enter()
{
    initNodes();

    if (!(GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library", 36L)))
	leave(20, "Requires V36 graphics.library");

    if (!(IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library", 36L)))
	leave(20, "Requires V36 intuition.library");

    if (!(GadToolsBase = OpenLibrary("gadtools.library", 36L)))
	leave(20, "Requires V36 gadtools.library");
}

/*------------------------------------------------------------------------*/

/*/ leave()
 *
 * Function to close down or free any opened or allocated stuff, and then
 * exit.
 *
 */

void leave(code, error)

int code;
STRPTR error;

{
    close_down();

    AllScreensDown();

    if (GadToolsBase)
    {
	CloseLibrary(GadToolsBase);
    }

    if (IntuitionBase)
    {
	CloseLibrary((struct Library *) IntuitionBase);
    }

    if (GfxBase)
    {
	CloseLibrary(GfxBase);
    }

    if (error)
    {
	printf("Error: %s\n", error);
    }
    exit(code);
}


/*------------------------------------------------------------------------*/

/*/ HandleGadgetEvent()
 *
 * Function to handle a GADGETUP or GADGETDOWN event.  For toolkit gadgets,
 * it is possible to use this function to handle MOUSEMOVEs as well, with
 * little or no work.
 *
 */

BOOL HandleGadgetEvent(win, gad, code)

struct Window *win;
struct Gadget *gad;
UWORD code;

{
    BOOL do_rethink = TRUE;
    LONG error = 0;

    switch (gad->GadgetID)
    {
	case GAD_LISTVIEW:
	    selnode = findScreenNodeOrder(code);
	    do_rethink = FALSE;
	    break;

	case GAD_TOFRONT:
	    if (selnode)
	    {
		ScreenToFront( selnode->sn_Screen );
	    }
	    else
	    {
		error = NO_SCREEN_SELECTED;
	    }
	    break;

	case GAD_TOBACK:
	    if (selnode)
	    {
		ScreenToBack( selnode->sn_Screen );
	    }
	    else
	    {
		error = NO_SCREEN_SELECTED;
	    }
	    break;

	case GAD_NEWFRONT:
	    ScreenUp( FRONT, NULL );
	    break;

	case GAD_NEWBACK:
	    ScreenUp( BACK, NULL );
	    break;

	case GAD_CHILDFRONT:
	case GAD_CHILDBACK:
	    /* Can only create children of non-children we made */
	    if ( selnode )
	    {
		if ( selnode->sn_Ours )
		{
		    if ( !selnode->sn_Parent )
		    {
			ScreenUp( (gad->GadgetID == GAD_CHILDFRONT)?
			    FRONT : BACK, selnode );
		    }
		    else
		    {
			error = ALREADY_CHILD;
		    }
		}
		else
		{
		    error = SCREEN_NOT_OURS;
		}
	    }
	    else
	    {
		error = NO_SCREEN_SELECTED;
	    }
	    break;

	case GAD_CLOSE:
	    if ( selnode )
	    {
		if ( selnode->sn_Ours )
		{
		    if ( !selnode->sn_Children )
		    {
			ScreenDown( selnode );
		    }
		    else
		    {
			error = SCREEN_HAS_CHILDREN;
		    }
		}
		else
		{
		    error = SCREEN_NOT_OURS;
		}
	    }
	    else
	    {
		error =  NO_SCREEN_SELECTED;
	    }
	    break;

	case GAD_REFRESH:
	    break;
    }
    GT_SetGadgetAttrs( textgad, mywin, NULL,
	GTTX_Text, ErrorString[error],
	TAG_DONE );

    return(do_rethink);
}


/*------------------------------------------------------------------------*/

/*/ CreateAllGadgets()
 *
 * Here is where all the initialization and creation of toolkit gadgets
 * take place.  This function requires a pointer to a NULL-initialized
 * gadget list pointer.  It returns a pointer to the last created gadget,
 * which can be checked for success/failure.
 *
 */

struct Gadget *CreateAllGadgets(glistptr, vi, topborder)

struct Gadget **glistptr;
void *vi;
UWORD topborder;

{
    struct NewGadget ng;

    struct Gadget *gad;

    /* All the gadget creation calls accept a pointer to the previous
     * gadget, and link the new gadget to that gadget's NextGadget field.
     * Also, they exit gracefully, returning NULL, if any previous gadget
     * was NULL.  This limits the amount of checking for failure that
     * is needed.  You only need to check before you tweak any gadget
     * structure or use any of its fields, and finally once at the end,
     * before you add the gadgets.
     */

    /* We obligingly perform the following operation, required of
     * any program that uses the toolkit.  It gives the toolkit a
     * place to stuff context data:
     */
    gad = CreateContext(glistptr);

    /* Since the NewGadget structure is unmodified by any of the
     * CreateGadget() calls, we need only change those fields which
     * are different.
     */

    ng.ng_LeftEdge = 10;
    ng.ng_TopEdge = 4+topborder;
    ng.ng_Width = 300;
    ng.ng_Height = 80;
    ng.ng_GadgetText = NULL;
    ng.ng_TextAttr = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_GadgetID = GAD_LISTVIEW;
    ng.ng_Flags = NG_HIGHLABEL;

    DP(("New Listview; selected node is %lx, order %ld\n",
	selnode, (selnode) ? selnode->sn_Order : ~0));
    lvgad = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
	GTLV_Labels, &screens,
	GTLV_ShowSelected, NULL,
	GTLV_Selected, (selnode) ? selnode->sn_Order : ~0,
	TAG_DONE);

    ng.ng_TopEdge += gad->Height+16;
    ng.ng_Height = 12;
    ng.ng_Width = 148;
    ng.ng_GadgetText = "To Front";
    ng.ng_GadgetID++;
    ng.ng_Flags = 0;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_LeftEdge = 162;
    ng.ng_GadgetText = "To Back";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += gad->Height+4;
    ng.ng_LeftEdge = 10;
    ng.ng_GadgetText = "Open New Front";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_LeftEdge = 162;
    ng.ng_GadgetText = "Open New Back";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += gad->Height+4;
    ng.ng_LeftEdge = 10;
    ng.ng_GadgetText = "Add Child Front";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_LeftEdge = 162;
    ng.ng_GadgetText = "Add Child Back";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += gad->Height+4;
    ng.ng_LeftEdge = 10;
    ng.ng_GadgetText = "Close Screen";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_LeftEdge = 162;
    ng.ng_GadgetText = "Refresh List";
    ng.ng_GadgetID++;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += gad->Height+4;
    ng.ng_LeftEdge = 10;
    ng.ng_Width =  300;
    ng.ng_GadgetText = NULL;
    ng.ng_GadgetID++;
    textgad = gad = CreateGadget(TEXT_KIND, gad, &ng,
	GTTX_Border, TRUE,
	TAG_DONE);

    return(gad);
}

/*------------------------------------------------------------------------*/

#define NodeInUse(node)	((node)->sn_Node.ln_Pri)

/*------------------------------------------------------------------------*/

void freeNode( node )
struct ScreenNode *node;
{
    node->sn_Node.ln_Pri = 0;
    node->sn_Order = ~0;
    node->sn_Ours = FALSE;
    node->sn_Screen = NULL;
    node->sn_Parent = NULL;
    node->sn_ScreenNumber = 0;
    node->sn_ChildNumber = 0;
    node->sn_Children = 0;
}

/*------------------------------------------------------------------------*/

/* initNodes():  Mark all nodes as free */

void initNodes()
{
    int i;

    for (i = 0; i < NUMNODES; i++)
    {
	freeNode( &nodes[i] );
    }
}

/*------------------------------------------------------------------------*/

/* findFreeNode():  Returns a free node or NULL if none */

struct ScreenNode *findFreeNode()
{
    int i;

    for (i = 0; i < NUMNODES; i++)
    {
	if (!NodeInUse(&nodes[i]))
	{
	    nodes[i].sn_Node.ln_Pri = 1;
	    return( &nodes[i] );
	}
    }
    return( NULL );
}

/*------------------------------------------------------------------------*/

/* orderNodes():  (Re)makes supplied list in screen-order */

void orderNodes( list )
struct List *list;
{
    ULONG ilock = LockIBase(NULL);
    struct Screen *s;
    struct ScreenNode *node;
    WORD order = 0;

    NewList( list );
    for (s = IntuitionBase->FirstScreen; s; s = s->NextScreen )
    {
	if (node = findScreenNode( s ) )
	{
	    node->sn_Order = order++;
	    AddTail( list, (struct Node *)node );
	}
    }
    UnlockIBase( ilock );
}

/*------------------------------------------------------------------------*/

/* findScreenNode():  Returns existing node of a screen, or makes a new one */

struct ScreenNode *findScreenNode(s)
struct Screen *s;
{
    int i;
    struct ScreenNode *node;

    DP(("fSN: Looking for screen '%s'\n",s->Title));
    for (i=0; i < NUMNODES; i++)
    {
#ifdef DEBUGGING
	if (NodeInUse(&nodes[i]))
	{
	    DP(("Node %ld is screen '%s'\n", i,nodes[i].sn_Screen->Title));
	}
#endif
	if ( NodeInUse(&nodes[i]) && (nodes[i].sn_Screen == s) )
	{
	DP(("Reusing node %ld for screen '%s'\n", i, s->Title));
	    return( &nodes[i] );
	}
    }
    if ( node = findFreeNode() )
    {
	DP(("Making a new node for screen '%s'\n", s->Title));
	node->sn_Screen = s;
	node->sn_Node.ln_Name = s->Title;
    }
    return( node );
}

/*------------------------------------------------------------------------*/

/* findScreenNodeOrder():  Given an order number, find the ScreenNode */

struct ScreenNode *findScreenNodeOrder( order )
WORD order;
{
    int i;

    for (i = 0; i < NUMNODES; i++)
    {
	if (nodes[i].sn_Order == order)
	{
	    return( &nodes[i] );
	}
    }
    return( NULL );
}

/*------------------------------------------------------------------------*/

void ScreenUp( behind, parent )
int behind;
struct ScreenNode *parent;

{
    struct Screen *sc = NULL;
    struct Window *win = NULL;
    struct ScreenNode *node;
    int top = 0;

    if ( node = findFreeNode() )
    {
	if (parent)
	{
	    node->sn_Parent = parent;
	    parent->sn_Children++;
	    node->sn_ScreenNumber = ++parent->sn_ChildNumber;
	    sprintf( node->sn_Title, " Screen %ld's Child %ld", 
		parent->sn_ScreenNumber, node->sn_ScreenNumber );
	    top = parent->sn_ChildNumber*20;
	}
	else
	{
	    node->sn_ScreenNumber = ++ScreenNumber;
	    sprintf( node->sn_Title, "Screen %ld", node->sn_ScreenNumber );
	}

	if ( sc = OpenScreenTags( NULL,
	    SA_DisplayID, HIRESLACE_KEY,
	    SA_Depth, 2,
	    SA_Title, node->sn_Title,
	    SA_Pens, pens,
	    SA_Top, top,
	    SA_Height, 400-top,
	    SA_Attach, (parent) ? parent->sn_Screen : NULL,
	    SA_Behind, behind,
	    TAG_DONE ) )
	{
	    if ( win = OpenWindowTags( NULL,
		WA_Backdrop, TRUE,
		WA_Borderless, TRUE,
		WA_SmartRefresh, TRUE,
		WA_NoCareRefresh, TRUE,
		WA_CustomScreen, sc,
		TAG_DONE ) )
	    {
	        WORD a,b,i;

		node->sn_Screen = sc;
		node->sn_Ours = TRUE;
		node->sn_Node.ln_Name = sc->Title;

		SetAPen( win->RPort, ~0 );
		a = win->Width/2 - 10;
		b = win->Height/2 - 10;
		for (i = 0; i < 1; i++)
		{
		    DrawEllipse( win->RPort,
			win->LeftEdge + win->Width/2,
			win->TopEdge + win->Height/2,
			a,
			b);
		    a -= 16;
		    b -= 16;
		}
	    }
	    else
	    {
		CloseScreen( sc );
		freeNode( node );
	    }
	}
	else
	{
	    freeNode( node );
	}
    }
}
   
/*------------------------------------------------------------------------*/

void ScreenDown( node )
struct ScreenNode *node;
{
    BOOL shut = FALSE;
    struct ScreenNode *parent = node->sn_Parent;

    if ( ( mywin ) && ( mywin->WScreen == node->sn_Screen ) )
    {
	close_down();
	shut = TRUE;
    }

    CloseWindow( node->sn_Screen->FirstWindow );
    CloseScreen( node->sn_Screen );
    if ( parent )
    {
	parent->sn_Children--;
    }
    Remove( (struct Node *)node );
    freeNode( node );

    if (shut)
    {
	mysc = IntuitionBase->FirstScreen;
	open_up();
    }
}

/*------------------------------------------------------------------------*/

void AllScreensDown()
{
    int i;

    for ( i = 0; i < NUMNODES; i++ )
    {
#ifdef DEBUGGING
	if (NodeInUse(&nodes[i]))
	{
	    DP(("Node %ld: screen '%s', ours? %ld, Children: %ld\n",
		i, nodes[i].sn_Screen->Title, nodes[i].sn_Ours,
		nodes[i].sn_Children));
	}
#endif
	/* First close all our children or childless parents: */
	if ( NodeInUse(&nodes[i]) && ( nodes[i].sn_Ours ) && (nodes[i].sn_Children == 0 ) )
	{
	    DP(("Closing leaf screen %ld\n",i));
	    ScreenDown( &nodes[i] );
	}
    }
    for ( i = 0; i < NUMNODES; i++ )
    {
	/* Now everyone else */
	if ( NodeInUse(&nodes[i]) && nodes[i].sn_Ours )
	{
	    DP(("Closing parent screen %ld\n",i));
	    ScreenDown( &nodes[i] );
	}
    }
}

/*------------------------------------------------------------------------*/
