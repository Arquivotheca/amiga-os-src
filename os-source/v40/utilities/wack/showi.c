/* showi.c : wack "show intuition" functions.
 * $Id: showi.c,v 1.16 92/08/20 17:57:16 peter Exp $
 */

#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/devices.h"
#include "exec/tasks.h"
#include "exec/resident.h"
#include "exec/execbase.h"

#include "exec/semaphores.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"

#if 0
#include "intuall.h"
#undef printf
/***
#include "intuition.h"
#include "intuinternal.h"
#include "intuitionbase.h"
#include "screens.h"
***/

#else
#include "intuition/iprefs.h"
#include "intuition/sc.h"
#include "intuition/i.h"
#include "intuition/intuinternal.h"
#include "intuition/ifunctions.h"
#endif

extern APTR CurrAddr;
extern APTR SpareAddr;
extern struct Window *LastWindow;

/** Screens **/

ULONG dumpXScreen(addr, s)
APTR addr;
struct Screen *s;
{
    struct ViewPortExtra vpextra;
    struct PubScreenNode	psn;
    char	psname[MAXPUBSCREENNAME];

    printf("---------- Screen %lx ------------------------\n", addr);
    printf("Flags: %8lx\nNextScreen: %8lx\nFirstWindow: %8lx\n",
	s->Flags, s->NextScreen, s->FirstWindow);

    if (s->Title)
    {
	char	titlebuff[ 256 ];
	GetBlock( s->Title, titlebuff, sizeof( titlebuff ) );
	printf("\"%s\"\n", titlebuff );
    }

    printf("PSNode: %lx\n", XSC(s)->PSNode );
    if ( XSC(s)->PSNode )
    {
	GetBlock( XSC(s)->PSNode, &psn, sizeof psn );
	GetBlock( psn.psn_Node.ln_Name, psname, sizeof psname );
	printf("public name: %s\n", psname  );
    }

    printf("L/T/W/H: %d/%d/%d/%d\n",
	s->LeftEdge, s->TopEdge, s->Width, s->Height);
    printf("MouseX/Y:\t%d/%d\n", s->MouseX, s->MouseY);

    ShowAddress("ViewPort", CADDR(addr, s, &s->ViewPort));
    ShowAddress("RastPort", CADDR(addr, s, &s->RastPort));
    ShowAddress("LayerInfo", CADDR(addr, s, &s->LayerInfo));

    ShowAddress("VPExtra", XSC(s)->VPExtra);
    ShowAddress("BarLayer", s->BarLayer );

    {
	struct TextAttr	tattr;
	char	nameb[ 44 ];

	GetBlock( s->Font, &tattr, sizeof tattr );
	GetBlock( tattr.ta_Name, nameb, sizeof nameb );

	printf("Font (TAttr) at %lx,'%s' size: %d\n", s->Font, nameb, tattr.ta_YSize);
    }

    GetBlock( XSC(s)->VPExtra, &vpextra, sizeof vpextra);

    dumpRect("screen DClip\t", &XSC(s)->DClip);
    dumpRect("viewport DisplayClip", &vpextra.DisplayClip);

    printf("scale factors: %d/%d -\t",
	XSC(s)->ScaleFactor.X, XSC(s)->ScaleFactor.Y );
    printf("sprite factors: %d/%d\n",
	XSC(s)->SpriteFactor.X, XSC(s)->SpriteFactor.Y );
    printf("NaturalDRecord: %lx, NaturalMSpec: %lx, NaturalDisplayID %lx\n",
	XSC(s)->NaturalDRecord,
	XSC(s)->NaturalMSpec,
	XSC(s)->NaturalDisplayID );

    printf("DProperties %lx ModeStash %lx\n", 
	XSC(s)->DProperties,
	XSC(s)->VPModeStash );

/* missing:
	ColorMap, SpriteFactor
	VPModeStash
	SysImages
	SDraw stuff
	*/

    printf("\n");
    
    return(0);
}

SHOWSIMPLE(XScreen)

/* gadgets */

ULONG dumpGadget(addr, g)
APTR addr;
struct Gadget *g;
{
    printf("Gadget %lx, ID: %x", addr, g->GadgetID);
    if ( g->GadgetText )
    {
	struct IntuiText gtext;
	GetBlock( g->GadgetText, &gtext, sizeof( gtext ) );
	if ( gtext.IText )
	{
	    char	labelbuff[ 256 ];
	    GetBlock( gtext.IText, labelbuff, sizeof( labelbuff ) );
	    printf(", \"%s\"", labelbuff );
	}
    }
    printf("\n");
    ShowAddress("NextGadget", g->NextGadget);
    printf("L/T/W/H:\t\t%d/%d/%d/%d\n",
	g->LeftEdge, g->TopEdge, g->Width, g->Height);
    printf("Flags %x Activation %x GadgetType %x\n",
	g->Flags, g->Activation, g->GadgetType);
    printf("GadgetRender %lx, SelectRender %lx\n",
	g->GadgetRender, g->SelectRender);
    ShowAddress("SpecialInfo", g->SpecialInfo);
    ShowAddress("UserData", g->UserData);
    printf("\n");

    return (0);
}

SHOWSIMPLE(Gadget)

/* menu items */

ULONG dumpMenuItem(addr, i)
APTR addr;
struct MenuItem *i;
{
    ShowAddress("MenuItem", addr);
    ShowAddress("NextItem", i->NextItem);
    printf("L/T/W/H:\t\t%d/%d/%d/%d\n", i->LeftEdge, i->TopEdge, i->Width, i->Height);
    printf("Flags %x, MutualExclude %lx\n", i->Flags, i->MutualExclude);
    ShowAddress("ItemFill", i->ItemFill);
    ShowAddress("SelectFill", i->SelectFill);
    printf("Command: %x, NextSelect %x\n", i->Command, i->NextSelect);
    ShowAddress("SubItem", i->SubItem);
    printf("\n");

    return (0);
}

SHOWSIMPLE(MenuItem)

/* menus */
ULONG dumpMenu(addr, m)
APTR addr;
struct Menu *m;
{
#define NAMESIZE 100
    char   name[NAMESIZE];

    ShowAddress("Menu", addr);
    ShowAddress("NextMenu", m->NextMenu);
    printf("L/T/W/H:\t\t%d/%d/%d/%d\n", m->LeftEdge, m->TopEdge, m->Width, m->Height);
    printf("Flags\t\t%x\n", m->Flags);
    GetName (m->MenuName, name);
    printf("MenuName %s\t", name );
    ShowAddress("--", m->MenuName);
    ShowAddress("FirstItem", m->FirstItem);
    printf("JazzX/Y BeatX/Y %d/%d  %d/%d\n",m->JazzX,m->JazzY,m->BeatX,m->BeatY);
    printf("\n");

    return (0);
}

SHOWSIMPLE(Menu)

/* windows */
ULONG dumpXWindow(addr, w)
APTR addr;
struct Window *w;
{
    LastWindow = (struct Window *)addr;
    printf(" ----------- Window %lx -------------------\n", addr );

    printf("\tNextWindow %8lx\tTitle %8lx\n",
	w->NextWindow, w->Title);
    if (w->Title)
    {
	char	titlebuff[ 256 ];
	GetBlock( w->Title, titlebuff, sizeof( titlebuff ) );
	printf("\t\"%s\"\n", titlebuff );
    }

    printf("\tWScreen %8lx\tRPort %8lx\tFirstGadget %8lx\n",
	w->WScreen, w->RPort, w->FirstGadget);
    printf("\tL/T/W/H: %d/%d/%d/%d\n", w->LeftEdge, w->TopEdge,
	w->Width, w->Height);
    printf("\tMouse X/Y: %d/%d\n", w->MouseX, w->MouseY);
    printf("\tMin W/H %d/%d\n", w->MinWidth, w->MinHeight);
    printf("\tMax W/H %d/%d\n", w->MaxWidth, w->MaxHeight);
    printf("\tFlags %lx MoreFlags %lx\n", w->Flags, w->MoreFlags);
    printf("\tFirstRequest %lx\n", w->FirstRequest);
    printf("\tDMRequest %lx\n", w->DMRequest);
    printf("\tReqCount %d\n", w->ReqCount);
    printf("\tParent %8lx\tDescendant %8lx\n", w->Parent, w->Descendant);
    printf("\tIDCMPFlags %8lx\tWLayer %8lx\n", w->IDCMPFlags, w->WLayer);

    printf("\tborder l/t/r/b %d/%d/%d/%d\n",
    w->BorderLeft, w->BorderTop, w->BorderRight, w->BorderBottom);
    printf("\tBorderRPort %lx\n", w->BorderRPort);
    ShowAddress ("\tIDCMP user port", w->UserPort);
    ShowAddress("\tMenuStrip", w->MenuStrip);
    printf("\tMouseQueueLimit: %d\n", XWINDOW(w)->MouseQueueLimit );
    printf("\tMousePending: %d\n", XWINDOW(w)->MousePending );
    printf("\tRptQueueLimit: %d\n", XWINDOW(w)->RptQueueLimit );
    printf("\tRptPending: %d\n", XWINDOW(w)->RptPending );
    printf("\n");

    return (0);
}

SHOWSIMPLE(XWindow)

ULONG dumpRequester(addr, r)
APTR addr;
struct Requester *r;
{
    ShowAddress("Requester", addr);
    ShowAddress ("OlderRequest", r->OlderRequest);
    ShowAddress ("ReqLayer", r->ReqLayer);
    ShowAddress ("RWindow", r->RWindow);
    ShowAddress("ReqGadget", r->ReqGadget);
    printf("Flags %lx\n", r->Flags);
    dumpBox ("reqbox", &r->LeftEdge);
    printf("rel left/top %d %d\n", r->RelLeft, r->RelTop);

    return (0);
}


SHOWSIMPLE(Requester)

dumpBox(s, b)
UBYTE *s;
struct IBox *b;
{
    printf("[%s]\t", s);
    printf("\tl/t: %d, %d; w/h: %d, %d\n",
	b->Left, b->Top, b->Width, b->Height);
}


dumpGEnv(ge)
struct GListEnv *ge;
{
    printf("no such thing as GListEnv anymore\n");
#if 0
    ShowAddress ("Screen", ge->ge_Screen);
    ShowAddress ("Window", ge->ge_Window);
    ShowAddress ("Requester", ge->ge_Requester);
    ShowAddress ("RastPort", ge->ge_RastPort);
    ShowAddress ("Layer", ge->ge_Layer);
    ShowAddress ("GZZLayer", ge->ge_GZZLayer);
    printf("pens %x %x\n", ge->ge_Pens.DetailPen, ge->ge_Pens.BlockPen);
    dumpBox("Domain", &ge->ge_Domain);
    dumpBox("GZZDims", &ge->ge_GZZdims);
#endif
}

ShowGEnv(arg)
char *arg;
{
    struct GListEnv *ge;

    if (AddrArg(arg, &ge)) 
    {
	printf("\n");
	doMinNode(ge, sizeof (struct GListEnv), dumpGEnv);
    }
    else puts("\nbad syntax");
}

ShowISems()
{
    int i;
    struct SignalSemaphore *sem;
    struct IntuitionBase *ibase;

    if (!(ibase = (struct IntuitionBase *) FindBase("intuition.library")))
    {
	printf("\ncan't find intuition.library\n");
	return;
    }

    semHeader();

    sem = &ibase->ISemaphore[0];

    for ( i = 0; i < NUMILOCKS; i++ )
    {
	showSem( sem );
	sem++;
    }
}

ShowLayerInfoSems()
{
    struct IntuitionBase *ibase;
    struct Screen *screen;

    if (!(ibase = (struct IntuitionBase *) FindBase("intuition.library")))
    {
	printf("\ncan't find intuition.library\n");
	return;
    }

    for (screen = (struct Screen *) GetMemL(&ibase->FirstScreen); screen;
			screen = (struct Screen *) GetMemL(&screen->NextScreen))
    {
	Print ("\nScreen: %lx, LayerInfo: %lx\n", screen, &screen->LayerInfo);
	Print ("Locks: (Layer_Info.Lock first)");
	semHeader();
	showSem(&screen->LayerInfo.Lock);
	showSemList(&screen->LayerInfo.gs_Head);
    }
}




ULONG dumpPropInfo(addr, pi)
APTR addr;
struct PropInfo *pi;
{

    ShowAddress("PropInfo", addr);
    printf("\tflags %x, pots %x/%x\n", pi->Flags, pi->HorizPot, pi->VertPot);
    printf("\tbodies: %x/%x\n", pi->HorizBody, pi->VertBody);
    printf("\tCWidth/Height %d/%d, H/VPotRes %d/%d\n", 
	pi->CWidth, pi->CHeight, pi->HPotRes, pi->VPotRes);

    printf("\tL/T Borders %d/%d\n", pi->LeftBorder, pi->TopBorder);

    return (1);	/* make it stop */
}

SHOWSIMPLE(PropInfo)

ULONG dumpImage(addr, im)
APTR addr;
struct Image *im;
{
    ShowAddress("Image", addr);
    printf("\tL/T/W/H %d/%d/%d/%d\n",
	im->LeftEdge, im->TopEdge, im->Width, im->Height);
    printf("\tDepth %d, ImageData %lx\n", im->Depth, im->ImageData);
    printf("\tPlanePick/OnOff %d - %d\n", im->PlanePick, im->PlaneOnOff);

    return (1);
}

SHOWSIMPLE(Image)


ULONG dumpIntuiText(addr, it)
APTR addr;
struct IntuiText *it;
{
    ShowAddress("IntuiText", addr);
    if (it->IText)
    {
	char	textbuff[ 256 ];
	GetBlock( it->IText, textbuff, sizeof( textbuff ) );
	printf("\tText: \"%s\"\n", textbuff );
    }
    printf("\tL/T %d/%d\n",
	it->LeftEdge, it->TopEdge);
    printf("\tFrontPen %d, BackPen %d, DrawMode %d\n", it->FrontPen,
	it->BackPen, it->DrawMode);
    printf("\tITextFont at %lx\n", it->ITextFont);
    printf("\tNextText at %lx\n", it->NextText);

    return (1);
}

SHOWSIMPLE(IntuiText)

struct Thing {struct Thing *NextThing};

ULONG dumpThing(addr, p)
APTR addr;
struct Thing *p;
{
    if (addr == CurrAddr)
    {
	printf("* ");
    }
    else
    {
	printf("  ");
    }
    ShowAddress("Thing", addr);

    return (0);
}

SHOWSIMPLE(Thing)

dumpRect(s, r)
UBYTE *s;
struct Rectangle *r;
{
    printf("%s\t", s);
    printf("\tX: [%d/%d]\tY: [%d/%d]\n", r->MinX, r->MaxX, r->MinY, r->MaxY);
}

ShowAScreen()
{
    struct IntuitionBase IBase;
    APTR  ibase;

    if (ibase =  FindBase ("intuition.library"))
    {
	GetBlock(ibase, &IBase, sizeof IBase);
	SpareAddr = (APTR) IBase.ActiveScreen;
	printf (" at %08lx\n", IBase.ActiveScreen );
    }
    else
    {
	puts("\ncouldn't find intuition.library in exec library list.");
    }
}

ShowAWindow()
{
    struct IntuitionBase IBase;
    APTR  ibase;

    if (ibase =  FindBase ("intuition.library"))
    {
	GetBlock(ibase, &IBase, sizeof IBase);
	SpareAddr = (APTR) ( LastWindow = IBase.ActiveWindow );
	printf (" at %08lx\n", IBase.ActiveWindow );
    }
    else
    {
	puts("\ncouldn't find intuition.library in exec library list.");
    }
}

ShowFirstGad()
{
    struct Window win;

    if (LastWindow)
    {
	GetBlock(LastWindow, &win, sizeof win);
	SpareAddr = (APTR) win.FirstGadget;
	printf (" of window at %08lx is at %08lx\n", LastWindow, win.FirstGadget);
    }
    else
    {
	puts("\nof what window?");
    }
}

