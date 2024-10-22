/*****************************************************************************
* handle all tab stuff
*****************************************************************************/

#include "st.h"
#include "term.h"
#include "colors.h"
#include "gadgets.h"

#define REQHEIGHT 80
#define REQLEFT 0

#define CANCELWIDTH 50
#define CANCELHEIGHT 9
#define CANCELLEFT 500
#define CANCELTOP 30
#define OKWIDTH 50
#define OKHEIGHT 9
#define OKLEFT 40
#define OKTOP 30
#define TABGHEIGHT 16
#define TABGLEFT 0
#define TABGTOP 50
#define EVERY8WIDTH 50
#define EVERY8HEIGHT 9
#define EVERY8LEFT (OKLEFT+OKWIDTH+64)
#define EVERY8TOP OKTOP
#define EVERY10WIDTH 60
#define EVERY10HEIGHT 9
#define EVERY10LEFT (EVERY8LEFT+EVERY8WIDTH+64)
#define EVERY10TOP OKTOP
#define CLEARWIDTH 50
#define CLEARHEIGHT 9
#define CLEARLEFT (EVERY10LEFT+EVERY10WIDTH+64)
#define CLEARTOP OKTOP

extern struct TextAttr MyFont; /* the font to use */
extern struct TextFont *tf_vt100_128;
extern ULONG FontBase;
extern WORD row, column;
extern UBYTE numcolumns;
extern UWORD scr_width;

char *col80 = "topaz.font", *col128 = "vt128.font";
static char numtext[MAXCOLUMNS] = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012";
UBYTE tabs[MAXCOLUMNS], tabstmp[MAXCOLUMNS], fwidth;
struct TextAttr TabFont = {NULL,8,FS_NORMAL,FPF_ROMFONT | FPF_DISKFONT | FPF_DESIGNED};
struct Requester TabRequester; /* the requester */

SHORT TabReqxys[] = { /* Requester xy pairs */
	0,0,
	0,0,
	0,REQHEIGHT-1,
	0,REQHEIGHT-1,
	0,0
};

struct Border TabReqBorder = { /* border for the requester */
	0,0,				/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	TabReqxys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

struct IntuiText TabClearText = { /* text for CLEAR gadget */
	COLOR1,COLOR1,JAM1,4,1,&MyFont,"CLEAR",NULL};

SHORT TabClearxys[] = { /* CLEAR gadget xy pairs */
	0,0,
	CLEARWIDTH+1,0,
	CLEARWIDTH+1,CLEARHEIGHT+1,
	0,CLEARHEIGHT+1,
	0,0
};

struct Border TabClearBorder = { 	/* border for CLEAR gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	TabClearxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget TabClearGadget = { 	/* the CLEAR gadget */
	NULL,		/* ptr to next gadget */
	CLEARLEFT,CLEARTOP,CLEARWIDTH,CLEARHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&TabClearBorder,		/* gadget render */
	NULL,			/* select render */
	&TabClearText,		/* text for CLEAR */
	0,			/* mutual exclude */
	NULL,			/* special info */
	CLEAR,			/* ID */
	NULL			/* UserData */
};

struct IntuiText TabEvery10Text = { /* text for EVERY10 gadget */
	COLOR1,COLOR1,JAM1,2,1,&MyFont,"EVERY10",NULL};

SHORT TabEvery10xys[] = { /* EVERY10 gadget xy pairs */
	0,0,
	EVERY10WIDTH+1,0,
	EVERY10WIDTH+1,EVERY10HEIGHT+1,
	0,EVERY10HEIGHT+1,
	0,0
};

struct Border TabEvery10Border = { 	/* border for EVERY10 gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	TabEvery10xys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget TabEvery10Gadget = { 	/* the EVERY10 gadget */
	&TabClearGadget,		/* ptr to next gadget */
	EVERY10LEFT,EVERY10TOP,EVERY10WIDTH,EVERY10HEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&TabEvery10Border,		/* gadget render */
	NULL,			/* select render */
	&TabEvery10Text,		/* text for EVERY10 */
	0,			/* mutual exclude */
	NULL,			/* special info */
	EVERY10,			/* ID */
	NULL			/* UserData */
};

struct IntuiText TabEvery8Text = { /* text for EVERY8 gadget */
	COLOR1,COLOR1,JAM1,1,1,&MyFont,"EVERY8",NULL};

SHORT TabEvery8xys[] = { /* EVERY8 gadget xy pairs */
	0,0,
	EVERY8WIDTH+1,0,
	EVERY8WIDTH+1,EVERY8HEIGHT+1,
	0,EVERY8HEIGHT+1,
	0,0
};

struct Border TabEvery8Border = { 	/* border for EVERY8 gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	TabEvery8xys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget TabEvery8Gadget = { 	/* the EVERY8 gadget */
	&TabEvery10Gadget,		/* ptr to next gadget */
	EVERY8LEFT,EVERY8TOP,EVERY8WIDTH,EVERY8HEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&TabEvery8Border,		/* gadget render */
	NULL,			/* select render */
	&TabEvery8Text,		/* text for EVERY8 */
	0,			/* mutual exclude */
	NULL,			/* special info */
	EVERY8,			/* ID */
	NULL			/* UserData */
};

struct IntuiText TabGText = { /* message for the requester */
   COLOR1,COLOR1,JAM1,0,10,&TabFont,numtext,NULL};

SHORT TabGxys[] = { /* TAB gadget xy pairs */
	0,0,
	0,0,
	0,TABGHEIGHT+1,
	0,TABGHEIGHT+1,
	0,0
};

struct Border TabGBorder = { 	/* border for TAB gadget */
	0,0,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	TabGxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget TabGadget = { 	/* the TAB gadget */
	&TabEvery8Gadget,		/* ptr to next gadget */
	TABGLEFT,TABGTOP,0,TABGHEIGHT,	/* left,top,width,height */
	GADGHNONE,		/* Gad_Flags (what to do when selected) */
	RELVERIFY,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&TabGBorder,		/* gadget render */
	NULL,			/* select render */
	&TabGText,		/* text for OK */
	0,			/* mutual exclude */
	NULL,			/* special info */
	TABG,			/* ID */
	NULL			/* UserData */
};

struct IntuiText TabOkText = { /* text for OK gadget */
	COLOR1,COLOR1,JAM1,16,1,&MyFont,"OK",NULL};

SHORT TabOkxys[] = { /* OK gadget xy pairs */
	0,0,
	OKWIDTH+1,0,
	OKWIDTH+1,OKHEIGHT+1,
	0,OKHEIGHT+1,
	0,0
};

struct Border TabOkBorder = { 	/* border for OK gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	TabOkxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget TabOkGadget = { 	/* the OK gadget */
	&TabGadget,		/* ptr to next gadget */
	OKLEFT,OKTOP,OKWIDTH,OKHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&TabOkBorder,		/* gadget render */
	NULL,			/* select render */
	&TabOkText,		/* text for OK */
	0,			/* mutual exclude */
	NULL,			/* special info */
	OK,			/* ID */
	NULL			/* UserData */
};

struct IntuiText TabCancelText = { /* text for CANCEL gadget */
	COLOR1,COLOR1,JAM1,1,1,&MyFont,"CANCEL",NULL};

SHORT TabCancelxys[] = { /* CANCEL gadget xy pairs */
	0,0,
	CANCELWIDTH+1,0,
	CANCELWIDTH+1,CANCELHEIGHT+1,
	0,CANCELHEIGHT+1,
	0,0
};

struct Border TabCancelBorder = { 	/* border for CANCEL gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,		/* front,back,drawmode */
	5,			/* # of xy pairs */
	TabCancelxys,		/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget TabCancelGadget = { /* the CANCEL gadget */
	&TabOkGadget,		/* ptr to next gadget */
	CANCELLEFT,CANCELTOP,CANCELWIDTH,CANCELHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&TabCancelBorder,		/* gadget render */
	NULL,			/* special render */
	&TabCancelText,		/* text for CANCEL */
	0,			/* mutual exclude */
	NULL,			/* special info */
	CANCEL,			/* ID */
	NULL			/* UserData */
};

struct IntuiText TabReqMsg = { /* text for requester */
	COLOR1,COLOR1,JAM1,254,10,&MyFont,"Select TABS",NULL};

UWORD reqtop;

TabReq(w)
struct Window *w;
{
struct RastPort *rp = &(w->WScreen->RastPort);
struct IntuiMessage *msg;
struct Gadget *address;
UWORD id;
UBYTE i, xposn, done=FALSE;
UWORD REQWIDTH = scr_width;
UWORD TABGWIDTH = scr_width;

	reqtop = (w->Height - REQHEIGHT) / 2;
	InitRequester(&TabRequester); /* init requester */
	TabRequester.LeftEdge = REQLEFT;
	TabRequester.TopEdge = reqtop;
	TabRequester.Width = REQWIDTH;
	TabRequester.Height = REQHEIGHT;
	TabRequester.ReqGadget = &TabCancelGadget;
	TabRequester.ReqBorder = &TabReqBorder;
	TabRequester.ReqText = &TabReqMsg;
	TabRequester.BackFill = COLOR0;
	if (numcolumns==MINCOLUMNS) {
		fwidth = 8;
		numtext[80] = '\000';
		TabFont.ta_Name = col80;
		SetFont(rp,FontBase);
	}
	else if (tf_vt100_128) { /* if 132 font exists */
		fwidth = 5;
		numtext[80] = '1';
		TabFont.ta_Name = col128;
		SetFont(rp,tf_vt100_128);
	}
	else {
		NoFont(1); /* 132 */
		return;
	}
	TabReqxys[2] = REQWIDTH - 1;
	TabReqxys[4] = REQWIDTH - 1;
	TabGxys[2] = TABGWIDTH;
	TabGxys[4] = TABGWIDTH;
	TabGadget.Width = TABGWIDTH;
	Request(&TabRequester,w); /* put it up */

   while (!done) { /* handle gadgets */
      Wait(1 << w->UserPort->mp_SigBit); /* for for intuition */
      while (msg = (struct IntuiMessage *)GetIntuition(w)) { /* get all msgs */
         switch (msg->Class) {
			case REQSET:
				SetDrMd(rp,JAM2);
				SetAPen(rp,COLOR1);
				SetBPen(rp,COLOR0);
				for (i=0; i<MAXCOLUMNS; i++) {
					tabstmp[i] = tabs[i]; /* copy tabs */
					show_tab(w,rp,i); /* show status */
				}
				break;
         case GADGETUP:
            address = (struct Gadget *)msg->IAddress;
				switch (address->GadgetID) {
				case OK: for (i=0; i<MAXCOLUMNS; i++) tabs[i] = tabstmp[i];
               done = TRUE;
					break;
            case CANCEL: done = TRUE;
					break;
				case TABG: xposn = (msg->MouseX) / fwidth; /* calc posn */
					tabstmp[xposn] = !tabstmp[xposn]; /* flip status */
					show_tab(w,rp,xposn); /* show status */
	            break;
				case EVERY8: clear_all_tmp(w,rp); /* clear all tmp tabs */
				   for (i=8; i<MAXCOLUMNS; i += 8) {
						tabstmp[i] = TRUE;
						show_tab(w,rp,i); /* show status */
					}
					break;
				case EVERY10: clear_all_tmp(w,rp); /* clear all tmp tabs */
				   for (i=10; i<MAXCOLUMNS; i += 10) {
						tabstmp[i] = TRUE;
						show_tab(w,rp,i); /* show status */
					}
					break;
				case CLEAR: clear_all_tmp(w,rp); /* clear all tmp tabs */
					break;
	         default: break;
				}
			}
         ReplyMsg(msg); /* reply to the msg */
		}
	}
}

clear_all_tmp(w,rp) /* clear all tmp tabs */
struct Window *w;
struct RastPort *rp;
{
UBYTE i;
	for (i=0; i<MAXCOLUMNS; i++) {
		tabstmp[i] = FALSE; /* clear all */
		show_tab(w,rp,i); /* show status */
	}
}

show_tab(w,rp,x) /* show the tab status of column x */
struct Window *w;
struct RastPort *rp;
UBYTE x;
{
	if (x<numcolumns) { /* if displayable */
		Move(rp,x*fwidth,16 + TABGTOP + reqtop + w->BorderTop); /* posn cursor */
		Text(rp,(tabstmp[x] ? "T" : " "),1); /* display */
		if (x==0) {
			Move(rp,0,8 + TABGTOP + reqtop + w->BorderTop); /* posn cursor */
			Draw(rp,0,8 + TABGTOP + reqtop + w->BorderTop + TABGHEIGHT); /* fix */
		}
		else if (x==(numcolumns-1)) {
			Move(rp,scr_width-1,8 + TABGTOP + reqtop + w->BorderTop); /* posn cursor */
			Draw(rp,scr_width-1,8 + TABGTOP + reqtop + w->BorderTop + TABGHEIGHT); /* fix */
		}
	}
}

set_tab() /* set tab at current cursor position */
{
   read_cursor_posn();
   tabs[column-1] = TRUE;
}

clear_tab() /* clear tab at current cursor position */
{
   read_cursor_posn();
   tabs[column-1] = FALSE;
}

clear_all_tabs() /* clear all tab positions */
{
UBYTE i;
   for (i=0; i<MAXCOLUMNS; i++) tabs[i] = FALSE;
}

do_tab() /* tab over to next tab position */
{
UBYTE i;
   read_cursor_posn(); /* get current posn */
   if (column < numcolumns) { /* if not already at right edge of screen */
      i = column + 1; /* start at the next column */
      while (!tabs[i-1] && i < numcolumns) i++; /* find next tab position */
      set_cursor_posn(row,i); /* move the cursor there */
	}
}

init_tabs() /* set default tabs to every 8 chars */
{
UBYTE i;
   clear_all_tabs();
   for (i=8; i<MAXCOLUMNS; i += 8) tabs[i] = TRUE;
}
