/* refresh.c */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include <stdio.h>
#include <string.h>

extern struct TextAttr TOPAZ80;

#include "refresh.h"
#include "global.h"

#include "gadgets.h"
#include "windows.h"

#include "sure.h"

extern struct Library *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct GadToolsBase *GadToolsBase;
//
struct Remember *RKey = NULL;	// For LISTVIEW_KIND Gadgets
extern UWORD XtraTop;		// For offset value ofscreen text
void do_refresh2(struct Window *,struct IntuiText *,void(*)(struct RastPort *));
//
#define GetIMsg(x) ((struct IntuiMessage *) GetMsg(x))

/*
   Handle just about any screen.  We're re-using the previous window,
   merely removing it's gadgets, and changing the title.  Must make sure
   we restore the state of the window (other than refresh) when we're
   done.

   "handler" handles intuition events, return HANDLE_DONE for done,
   HANDLE_CANCEL for cancel, HANDLE_REFRESH if it wants the window/
   gadgets refreshed.  It may invoke another version of HandleScreen if
   it wishes.  REFRESHWINDOW is not sent to the handler.

   initrtn() is called BEFORE the gadgets are added to the window, but
   after the old ones are removed.  Useful for reseting prop gadget values,
   etc.  If initrtn returns FALSE, the window is aborted (error).
*/

struct IntuiText *global_itext;
void   (*global_refreshrtn)(struct RastPort *);

int		// New function for HandleWindow()
HandleWindowNew (w,itext,newglist,title,idcmpflags,handler,refreshrtn,initrtn,
			cragads_new, saveglist, cragads_save)
	register struct Window *w;
	struct IntuiText *itext;	/* window itext */
	struct Gadget **newglist;
	UBYTE  *title;
	ULONG  idcmpflags;		// 39.10
	int    (*handler)(struct Window *,struct IntuiMessage *);
							/* handles GadgetUp  */
	void   (*refreshrtn)(struct RastPort *);	/* draws into window */
	int    (*initrtn)(struct Window *);		/* called once */
	struct Gadget *(*cragads_new)(void);
	struct Gadget **saveglist;
	struct Gadget *(*cragads_save)(void);
{
	register struct IntuiMessage *msg;
	UBYTE *oldTitle;
	ULONG oldIDCMPFlags;		// 39.10
	register int done = FALSE;
	struct IntuiText *save_itext;
	void   (*save_refreshrtn)(struct RastPort *);
	ULONG imsgClass;
	struct Gadget *FirstGad;

	if (w)	/* paranoia */
	{
		save_itext = global_itext;
		save_refreshrtn = global_refreshrtn;
		global_itext = itext;
		global_refreshrtn = refreshrtn;

//		sprintf(tmp, "%x", *saveglist);
//		kprintf("remove old gad : %s\n",tmp);

		/* Remove old gadgets */
		if (*saveglist)
		{
			FirstGad = w->FirstGadget;
			while ((FirstGad) && (FirstGad->GadgetType & SYSGADGET))
				FirstGad = FirstGad->NextGadget;
			if (FirstGad)
			{
				RemoveGList(w,FirstGad,-1L);
				FreeGadgets(FirstGad);
				*saveglist = NULL;
			}
		}

		/* If we use LISTVIEW_KIND gadgets, we have to free list */
		if (RKey)
			{
//			sprintf(tmp, "%x", RKey);
//			kprintf("remove old Rkey : %s\n",tmp);
			FreeRemember(&RKey,TRUE);
			}

		/* before drawing anything or adding gads, do init rtn */
		if (initrtn)
			if (((*initrtn)(w)) == FALSE)
				goto cleanup;

		/* Create new gadgets */
		if (cragads_new)
			if (((*cragads_new)()) == NULL)
				goto cleanup;
//		sprintf(tmp, "%x", *newglist);
//		kprintf("create new gad : %s\n",tmp);

		AddGList(w,*newglist,-1L,-1L,(struct Requester *) NULL);

		oldTitle = w->Title;
		SetWindowTitles(w,title,(UBYTE *) -1L);
		oldIDCMPFlags = w->IDCMPFlags;	// 39.10
		ModifyIDCMP(w,idcmpflags);	// 39.10

		do_refresh(w,itext,refreshrtn);		/* draw window */

		while (!done) {
			Wait(1<<w->UserPort->mp_SigBit);
			while (msg = GT_GetIMsg(w->UserPort)) {

				imsgClass = msg->Class;

				GT_ReplyIMsg(msg);  /* return */

				if (imsgClass == REFRESHWINDOW)
				{
//					kprintf("refresh window\n");
					GT_BeginRefresh(w);

					if(itext || refreshrtn)	//MMM
						do_refresh2(w,itext,refreshrtn);
				/*	RefreshWindowFrame(w); 		*/

					GT_EndRefresh(w,TRUE);
				} else {
					if ((done = (*handler)(w,msg)) ==
					    HANDLE_REFRESH)
					{
//						kprintf("another refresh window\n");
						done = 0;
						GT_BeginRefresh(w);
						GT_EndRefresh(w,TRUE);
						do_refresh(w,itext,refreshrtn);
					}
				}

				if (done)
					break;
			}
		}

//		sprintf(tmp, "%x", *newglist);
//		kprintf("remove new gad : %s\n",tmp);
		/* Remove new gadgets */
		if (*newglist)
		{
			FirstGad = w->FirstGadget;
			while ((FirstGad) && (FirstGad->GadgetType & SYSGADGET))
				FirstGad = FirstGad->NextGadget;
			if (FirstGad)
			{
				RemoveGList(w,FirstGad,-1L);
				FreeGadgets(FirstGad);
				*newglist = NULL;
			}
		}
		/* If we use LISTVIEW_KIND gadgets, we have to free list */
		if (RKey)
			{
//			sprintf(tmp, "%x", RKey);
//			kprintf("remove new Rkey : %s\n",tmp);
			FreeRemember(&RKey,TRUE);
			}

		/* restore old window characteristics */
cleanup:				/* initrtn failed */
		if (cragads_save)
			{
			(*cragads_save)();
			AddGList(w,*saveglist,-1L,-1L,(struct Requester *) NULL);
			}

//		sprintf(tmp, "%x", *saveglist);
//		kprintf("create save gad : %s\n",tmp);
		SetWindowTitles(w,oldTitle,(UBYTE *) -1L);
		ModifyIDCMP(w,oldIDCMPFlags);	// 39.10

		/* restore globals used by handle_requester */
		global_itext = save_itext;
		global_refreshrtn = save_refreshrtn;
	}

	return HANDLE_REFRESH;
}

#if 0
int
HandleWindow (w,itext,newgadget,title,handler,refreshrtn,initrtn)
	register struct Window *w;
	struct IntuiText *itext;	/* window itext */
	struct Gadget *newgadget;
	UBYTE  *title;
	int    (*handler)(struct Window *,struct IntuiMessage *);
							/* handles GadgetUp  */
	void   (*refreshrtn)(struct RastPort *);	/* draws into window */
	int    (*initrtn)(struct Window *);		/* called once */
{

	register struct IntuiMessage *msg,mmsg;
	struct Gadget *savegad;
	UBYTE *oldTitle;
	register int done = FALSE;
	struct IntuiText *save_itext;
	void   (*save_refreshrtn)(struct RastPort *);

	if (w)	/* paranoia */
	{
		save_itext = global_itext;
		save_refreshrtn = global_refreshrtn;
		global_itext = itext;
		global_refreshrtn = refreshrtn;

		/* ASSUMPTION: sys gadgets at front of gadget list! */
		for (savegad = w->FirstGadget;
		     savegad && (savegad->GadgetType & SYSGADGET);
		     savegad = savegad->NextGadget);

		if (savegad)
			RemoveGList(w,savegad,-1L);	/* remove old gads */

		/* before drawing anything or adding gads, do init rtn */
		if (initrtn)
			if (((*initrtn)(w)) == FALSE)
				goto cleanup;
		AddGList(w,newgadget,-1L,-1L,(struct Requester *) NULL);

		oldTitle = w->Title;
		SetWindowTitles(w,title,(UBYTE *) -1L);

		do_refresh(w,itext,refreshrtn);		/* draw window */

		while (!done) {
			WaitPort(w->UserPort);
			while (msg = GetIMsg(w->UserPort)) {

				mmsg = *msg;	/* copy */
				ReplyMsg((struct Message *) msg);  /* return */

				msg = &mmsg;

				if (msg->Class == REFRESHWINDOW)
				{
					BeginRefresh(w);

					do_refresh(w,itext,refreshrtn);
				/*	RefreshWindowFrame(w); 		*/

					EndRefresh(w,TRUE);

				} else {
					if ((done = (*handler)(w,msg)) ==
					    HANDLE_REFRESH)
					{
						done = 0;
						BeginRefresh(w);
						EndRefresh(w,TRUE);
						do_refresh(w,itext,refreshrtn);
					}
				}

				if (done)
					break;
			}
		}

		/* restore old window characteristics */
		RemoveGList(w,newgadget,-1L);

cleanup:				/* initrtn failed */
		if (savegad)
			AddGList(w,savegad,-1L,-1L,(struct Requester *) NULL);


		SetWindowTitles(w,oldTitle,(UBYTE *) -1L);

		/* restore globals used by handle_requester */
		global_itext = save_itext;
		global_refreshrtn = save_refreshrtn;
	}

	return HANDLE_REFRESH;
}
#endif

/* put up requesters and handle them */

int
HandleRequest (w,req,handler)
	register struct Window *w;
	struct Requester *req;
	int    (*handler)(struct Window *,struct Requester *,
			  struct IntuiMessage *);
							/* handles GadgetUp  */
{
	register struct IntuiMessage *msg;
	register int done = FALSE;

	if (!Request(req,w)) return HANDLE_DONE;

	while (!done) {
		WaitPort(w->UserPort);
		while (msg = GetIMsg(w->UserPort)) {

			if (msg->Class == REFRESHWINDOW)
			{
				BeginRefresh(w);

				do_refresh(w,global_itext,global_refreshrtn);

				EndRefresh(w,TRUE);
			}
			if (handler) {

				done = (*handler)(w,req,msg);
                                }

			if (msg->Class == REQCLEAR)
				done = HANDLE_REFRESH;

			ReplyMsg((struct Message *) msg);

			if (done)
				break;
		}

		/* if user wants to remove it, he must set ENDGADGET or */
		/* do an EndRequest(req,window)				*/
		/* returning Handle_Done/Handle_Cancel should NOT be    */
		/* mixed with ENDGADGET!!				*/
	}

	if (done != HANDLE_REFRESH)
	{
		EndRequest(req,w);

		/* Eat the REQCLEAR message */
		while (1) {
			WaitPort(w->UserPort);
			msg = (struct IntuiMessage *) GetMsg(w->UserPort);
			if (msg->Class == REQCLEAR)
			{
				ReplyMsg((struct Message *) msg);
				break;
			}
			ReplyMsg((struct Message *) msg);
		}

#ifdef foob
		/* more funky magic: to make simple refresh windows look */
		/* nice, search for a REFRESHWINDOW message and eat it   */

		while (msg = (struct IntuiMessage *) GetMsg(w->UserPort))
		{
			if (msg->Class == REFRESHWINDOW)
			{
				BeginRefresh(w);
				do_refresh(w,global_itext,global_refreshrtn);
				EndRefresh(w,TRUE);
				ReplyMsg((struct Message *) msg);
				break;
			}
			ReplyMsg((struct Message *) msg);
		}
#endif foob
	} else
		done = HANDLE_DONE;

	return done;
}

void
do_refresh(w,itext,refreshrtn)
	register struct Window *w;
	struct IntuiText *itext;		/* window itext */
	void   (*refreshrtn)(struct RastPort *);	/* draws into window */
{
	/* these colors can be "stacked" at each invocation of HandleWindow */
	refresh(w,colors[0],colors[1],colors[2],colors[3]);

	if (itext)
		PrintIText(w->RPort,itext,0,0);
	if (refreshrtn)
		(*refreshrtn)(w->RPort);
}

// This routine is for only "Partitioning" window, because only this window has
// 1.3 type gadgets. If we use do_refresh(), it calls refresh(), and it calls
// GT_RefreshWindow(), it cause a lot of rendering.
void
do_refresh2(w,itext,refreshrtn)
	register struct Window *w;
	struct IntuiText *itext;		/* window itext */
	void   (*refreshrtn)(struct RastPort *);	/* draws into window */
{
	if (itext)
		PrintIText(w->RPort,itext,0,0);
	if (refreshrtn)
		(*refreshrtn)(w->RPort);
}

/* Notify the user of something: open a requester with a single gadget */

void
Notify (w,str,parm)
	struct Window *w;
	char *str;
	LONG parm;
{
	LONG width;
//	char temp[81];
	char temp[200];	// It's possible that string length is over 81.
			// See MakeFileSys() in handlefsm.c

	sprintf(temp,str,parm);
	NotifyIText.IText = temp;

	width = IntuiTextLength(&NotifyIText);
	NotifyRequester.LeftEdge = max((640 - width - 20) >> 1,0);
	NotifyRequester.Width    = 640 - (NotifyRequester.LeftEdge << 1);
	NotifyGad.LeftEdge	 = (NotifyRequester.Width >> 1) - 
				   (NotifyGad.Width >> 1);
//	Make border for requester
	MakePrettyRequestBorder(NotifyRequester.Width,NotifyRequester.Height);

	/* make border? */

	HandleRequest(w,&NotifyRequester,NULL);
}

/* make sure the user wants to do something */

int
SureHandler (w,req,msg)
	struct Window *w;
	struct Requester *req;
	struct IntuiMessage *msg;
{
	int done = 0;

	if (msg->Class == GADGETUP)
		if (msg->IAddress == (APTR) &SureContinueGad)
			done = HANDLE_DONE;
		else if (msg->IAddress == (APTR) &SureCancelGad)
			done = HANDLE_CANCEL;
		else
			; /* Huh? */
	return done;
}

int
AskSure (w,str)
	struct Window *w;
	register char **str;	/* like an argv array, but terminated by NULL */
{
	return InternalAskSure(w,str,1);
}

void
LongNotify (w,str)
	struct Window *w;
	register char **str;	/* like an argv array, but terminated by NULL */
{
	(void) InternalAskSure(w,str,0);
}

int
InternalAskSure (w,str,flag)
	struct Window *w;
	register char **str;	/* like an argv array, but terminated by NULL */
	int flag;		/* 1 = SureHandler, 2 = NotifyHandler */
{
	register struct IntuiText *it;
		 struct IntuiText *lastit;
	register LONG width;
		 int  res;

	it = &SureIText[0];
	width = 120;

	/* set up the ITexts */
	while (*str && strlen(*str)) {
		it->IText = *str;
		width = max(IntuiTextLength(it) + 20,width);
		str++;
		if (*str && strlen(*str))
		{
			it->NextText = it + 1;	/* plus 1 IText */
			it++;			/* avoid order of eval probs */
		}
	}
	it->NextText = NULL;
	lastit = it;

	/* now center the text */
	for (it = &SureIText[0]; it; it = it->NextText) {
		it->LeftEdge = (width - IntuiTextLength(it)) >> 1;
	}

	AskSure_Requester.LeftEdge = max((640 - width) >> 1,0);
	AskSure_Requester.Width    = 640 - (AskSure_Requester.LeftEdge << 1);
//	AskSure_Requester.TopEdge  = 100 - (lastit->TopEdge + 30)/2;
	AskSure_Requester.TopEdge  = 100 - (lastit->TopEdge + 30)/2 + XtraTop;
//	AskSure_Requester.Height   = lastit->TopEdge + 30;
	AskSure_Requester.Height   = lastit->TopEdge + 34;
	AskSure_Requester.ReqText  = &SureIText[0];

//	Make border for requester
	MakePrettyRequestBorder(AskSure_Requester.Width,AskSure_Requester.Height);

	if (flag == 1)
	{
		AskSure_Requester.ReqGadget = &SureContinueGad;
	} else {
		AskSure_Requester.ReqGadget = &NotifyGad;
		NotifyGad.LeftEdge	    = (AskSure_Requester.Width >> 1) - 
					      (NotifyGad.Width >> 1);
	}

	/* make border? */

	res = HandleRequest(w,&AskSure_Requester,
			    flag ? SureHandler : 0);

	return (res == HANDLE_DONE);
}

/* refresh()
 *
 *	This code will refresh a prep (prefs-style) window.  It will draw
 * a background, then reset any area to be covered by a gadget.
 *
 * 	It may be safe to call between Begin/EndRefresh().
 *
 *	Does not currently work with any GRELxxx gadgets.
 *
 *	Modifies the DrawMode and Pen states of the windows rastport.
 * 	(but won't leave it in AOLine)
 *
 *	if gadgetid has BORDER, it will draw a border for the gadgets
 *	if gadgetid has DROPSHADOW, it will draw a dropshadow for the gadgets
 *	if gadgetid has FANCY, it will draw a foncy border for the gadgets
 *	if gadgetid has DONTDRAW, it won't do anything
 *
 *	it ignores system gadgets
 */

static WORD poly[18];

static struct Border border = {
	0,0,
	0,0,
	JAM1,
	0,
	&poly[0],
	NULL,
};

void
refresh (w,background,gadcolor,bordercol,shadow)
	struct Window *w;
	ULONG background,gadcolor,bordercol,shadow;
{
	register struct RastPort *rp;
#if 0
	register struct Gadget   *gad;
	register UWORD ID;
#endif	register WORD width,height;

	rp = w->RPort;

	/* first clear everything out */
	SetAPen(rp,background);
	SetDrMd(rp,JAM1);
	SetAfPt(rp,NULL,0);
	RectFill(rp,w->BorderLeft,w->BorderTop,
		 w->Width - w->BorderRight - 1,w->Height - w->BorderBottom - 1);

	/* now fill area above sizing gadget if there is one */
	if (w->Flags & WINDOWSIZING)
	{
		RectFill(rp,w->Width - w->BorderRight,w->BorderTop,
			    w->Width - w->BorderLeft,
			    w->Height - w->BorderBottom - 9);	/* voodoo */
	}

#if 0
	/* now walk the gadget list, clearing as needed */
	gad = w->FirstGadget;
	while (gad) {
		ID = gad->GadgetID;

		if (!(gad->GadgetType & SYSGADGET))
		{
			width  = gad->Width;
			height = gad->Height;

			if (!(ID & DONTDRAW))
			{
				SetAPen(rp,gadcolor);
				RectFill(rp,gad->LeftEdge, gad->TopEdge,
					    gad->LeftEdge + width  - 1,
					    gad->TopEdge  + height - 1);
			}
			if (ID & BORDER)
			{
				border.FrontPen = bordercol;
				border.LeftEdge = gad->LeftEdge-1;
				border.TopEdge  = gad->TopEdge-1;
				border.Count    = 5;

				poly[0] = poly[1] =
				poly[2] = poly[7] =
				poly[8] = poly[9] = 0;
				poly[3] = poly[5] = height + 1;
				poly[4] = poly[6] = width + 1;

				DrawBorder(rp,&border,0,0);
			}

			/* draw dropshadow */
			if (ID & (DROPSHADOW | FANCY)) {
				border.LeftEdge = gad->LeftEdge + 3;
				border.TopEdge  = gad->TopEdge + height + 1;
				border.FrontPen = shadow;

				if (ID & DROPSHADOW) {
					if (!(ID & BORDER)) {
						border.LeftEdge--;
						border.TopEdge--;
					}
					border.Count = 5;

					poly[2] = poly[4] = width - 1;
					poly[6] = poly[8] = width;
					poly[5] = poly[7] = -height;
					poly[3] = poly[9] =
					poly[0] = poly[1] = 0;

				} else if (ID & FANCY) {
					border.Count = 9;
					border.LeftEdge += 1;

					poly[2]  = poly[8]  = width - 9;
					poly[4]  = poly[6]  = width - 1;
					poly[12] = poly[14] = -8;
					poly[5]  = poly[15] = -3;
					poly[7]  = poly[13] = -height;
					poly[9]  = poly[11] = -height - 3;
					poly[16] = poly[17] =
					poly[10] = poly[3]  =
					poly[0]  = poly[1]  = 0;
				}
				DrawBorder(rp,&border,0,0);
			}
			if (ID & BORDER)
				BNDRYOFF(rp);	/* turn off area outline */
		}
		gad = gad->NextGadget;	
	}
#endif
	/* now refesh all the gadgets */

	RefreshGList(w->FirstGadget,w,NULL,-1L);

	GT_RefreshWindow(w,NULL);	// for Gadgettools
}

#ifdef TEST
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

struct IntuiText itext = {
	1,0,JAM1,12,2,NULL,"a test",NULL,
};

struct Gadget gad = {
	NULL,	/* next gadget */
	84,57,	/* origin XY of hit box relative to window TopLeft */
	79,12,	/* hit box width and height */
	GADGHCOMP,	/* gadget flags */
	RELVERIFY|TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	NULL,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&itext,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	BORDER|DROPSHADOW,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

struct NewWindow newwindow = {
	0,0,640,200,
	0,1,
	CLOSEWINDOW|RAWKEY|MOUSEBUTTONS,
	WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|SMART_REFRESH|NOCAREREFRESH|ACTIVATE,
	&gad,
	NULL,
	"my window",
	NULL,
	NULL,
	0,0,-1,-1,
	WBENCHSCREEN
};

ULONG pars[] = { 2,0,1,1};
UWORD flags[5] = {
	BORDER,
	BORDER|DROPSHADOW,
	BORDER|FANCY,
	DROPSHADOW,
	FANCY,
};

main(argc,argv)
	int argc;
	char **argv;
{
	struct Window *w;
	struct IntuiMessage *msg;
	int done = FALSE;
	int i = 0;

	while (i < argc - 1) {
		stcd_i(argv[i+1],&pars[i]);
		i++;
	}
	IntuitionBase = OpenLibrary("intuition.library",0L);
	GfxBase = OpenLibrary("graphics.library",0L);

	w = OpenWindow(&newwindow);
	if (w) {
		i = 0;
		while (!done) {
			WaitPort(w->UserPort);
			while (msg = GetMsg(w->UserPort)) {
				if (msg->Class == CLOSEWINDOW)
					done = TRUE;
				else if (msg->Class == MOUSEBUTTONS) {
					SetRast(w->RPort,3L);
					RefreshWindowFrame(w);
					if (++i > 4)
						i = 0;
				} else {
					gad.GadgetID = flags[i];
					refresh(w,pars[0],pars[1],pars[2],
						  pars[3]);
				}
				ReplyMsg(msg);
			}
		}
		CloseWindow(w);
	}
	CloseLibrary(IntuitionBase);
	CloseLibrary(GfxBase);
	exit(0);
}

#endif
