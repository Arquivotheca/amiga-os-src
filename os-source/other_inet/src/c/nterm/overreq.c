/*****************************************************************************
* OVERwrite requester
*****************************************************************************/

#include "st.h"
#include "colors.h"
#include "term.h"

#define OVERCANCEL 0
#define OVEROK 1

#define OVERREQWIDTH 196
#define OVERREQHEIGHT 48

#define OVERCANCELWIDTH 50
#define OVERCANCELHEIGHT 9
#define OVERCANCELLEFT (OVERREQWIDTH-OVERCANCELWIDTH-16)
#define OVERCANCELTOP (OVERREQHEIGHT-OVERCANCELHEIGHT-8)
#define OVEROKWIDTH 50
#define OVEROKHEIGHT 9
#define OVEROKLEFT 16
#define OVEROKTOP OVERCANCELTOP

#define RP (&(w->WScreen->RastPort))

extern struct TextAttr MyFont; /* the font to use */

struct Requester OverRequester; /* the requester */

SHORT OverReqxys[] = { /* Requester xy pairs */
	0,0,
	OVERREQWIDTH-1,0,
	OVERREQWIDTH-1,OVERREQHEIGHT-1,
	0,OVERREQHEIGHT-1,
	0,0
};

struct Border OverReqBorder = { /* border for the requester */
	0,0,				/* left,top */
	COLOR1,COLOR0,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	OverReqxys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

char overstr[] = "Overwrite file?"; /* the title string */
struct IntuiText OverText = { /* text for over title */
   COLOR1,COLOR0,JAM1,0,12,&MyFont,overstr,NULL};

struct IntuiText OverOkText = { /* text for the OK gadget */
	COLOR1,COLOR0,JAM1,0,1,&MyFont,"  OK  ",NULL};

SHORT OverOkxys[] = { /* OK gadget xy pairs */
	0,0,
	OVEROKWIDTH+1,0,
	OVEROKWIDTH+1,OVEROKHEIGHT+1,
	0,OVEROKHEIGHT+1,
	0,0
};

struct Border OverOkBorder = { 	/* border for OK gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR0,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	OverOkxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget OverOkGadget = { 	/* the OK gadget */
	NULL,		/* ptr to next gadget */
	OVEROKLEFT,OVEROKTOP,OVEROKWIDTH,OVEROKHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&OverOkBorder,		/* gadget render */
	NULL,			/* select render */
	&OverOkText,		/* text for OVEROK */
	0,			/* mutual exclude */
	NULL,			/* special info */
	OVEROK,			/* ID */
	NULL			/* UserData */
};

struct IntuiText OverCancelText = { /* text for CANCEL gadget */
	COLOR1,COLOR0,JAM1,1,1,&MyFont,"CANCEL",NULL};

SHORT OverCancelxys[] = { /* CANCEL gadget xy pairs */
	0,0,
	OVERCANCELWIDTH+1,0,
	OVERCANCELWIDTH+1,OVERCANCELHEIGHT+1,
	0,OVERCANCELHEIGHT+1,
	0,0
};

struct Border OverCancelBorder = { 	/* border for CANCEL gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR0,JAM1,		/* front,back,drawmode */
	5,			/* # of xy pairs */
	OverCancelxys,		/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget OverCancelGadget = { /* the CANCEL gadget */
	&OverOkGadget,		/* ptr to next gadget */
	OVERCANCELLEFT,OVERCANCELTOP,OVERCANCELWIDTH,OVERCANCELHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&OverCancelBorder,		/* gadget render */
	NULL,			/* special render */
	&OverCancelText,		/* text for OVERCANCEL */
	0,			/* mutual exclude */
	NULL,			/* special info */
	OVERCANCEL,			/* ID */
	NULL			/* UserData */
};

OverReq(w)
struct Window *w;
{
extern UWORD scr_width;

struct IntuiMessage *msg;
struct Gadget *address;
UWORD id, overreqtop;
UBYTE i, done=FALSE;
UWORD OVERREQLEFT = ((scr_width-OVERREQWIDTH)/2);

	overreqtop = (w->Height - OVERREQHEIGHT) / 2;
	InitRequester(&OverRequester); /* init requester */
	OverRequester.LeftEdge = OVERREQLEFT;
	OverRequester.TopEdge = overreqtop;
	OverRequester.Width = OVERREQWIDTH;
	OverRequester.Height = OVERREQHEIGHT;
	OverRequester.ReqGadget = &OverCancelGadget;
	OverRequester.ReqBorder = &OverReqBorder;
	OverRequester.ReqText = &OverText;
	OverRequester.BackFill = COLOR0;
	OverText.LeftEdge = (OVERREQWIDTH-strlen(overstr)*8)/2; /* center title */
	Request(&OverRequester,w); /* put it up */
   while (!done) { /* handle gadgets */
      Wait(1 << w->UserPort->mp_SigBit); /* for for intuition */
      while (msg = (struct IntuiMessage *)GetIntuition(w)) { /* get all msgs */
         switch (msg->Class) {
			case GADGETUP:
            address = (struct Gadget *)msg->IAddress;
            id = address->GadgetID;
				switch (id) {
				case OVEROK: case OVERCANCEL: done = TRUE;
					break;
	         default: break;
				}
				break;
         default: break;
			}
         ReplyMsg(msg); /* reply to the msg */
		}
	}
	return (id==OVEROK);
}

