/*****************************************************************************
* do all the change color slider requester stuff
*****************************************************************************/

#include "st.h"
#include "term.h"
#include "colors.h"

/*#define DEBUG /* */

#define COLORREQWIDTH 480
#define COLORREQHEIGHT 80

#define COLORCANCELWIDTH 50
#define COLORCANCELHEIGHT 9
#define COLORCANCELLEFT (COLORREQWIDTH-COLORCANCELWIDTH-16)
#define COLORCANCELTOP (COLORREQHEIGHT-COLORCANCELHEIGHT-8)
#define COLOROKWIDTH 50
#define COLOROKHEIGHT 9
#define COLOROKLEFT 16
#define COLOROKTOP COLORCANCELTOP
#define COLORRESETWIDTH 50
#define COLORRESETHEIGHT 9
#define COLORRESETLEFT ((COLORREQWIDTH-COLORRESETWIDTH)/2)
#define COLORRESETTOP COLORCANCELTOP
#define COLORBACKWIDTH 82
#define COLORBACKHEIGHT 9
#define COLORBACKLEFT ((COLORREQWIDTH-COLORBACKWIDTH)/2)
#define COLORBACKTOP 8
#define COLORREDWIDTH (COLORREQWIDTH-64)
#define COLORREDHEIGHT 9
#define COLORREDLEFT 50
#define COLORREDTOP (COLORBACKTOP+COLORBACKHEIGHT+8)
#define COLORGREENWIDTH COLORREDWIDTH
#define COLORGREENHEIGHT 9
#define COLORGREENLEFT COLORREDLEFT
#define COLORGREENTOP (COLORREDTOP+COLORREDHEIGHT+1)
#define COLORBLUEWIDTH COLORREDWIDTH
#define COLORBLUEHEIGHT 9
#define COLORBLUELEFT COLORREDLEFT
#define COLORBLUETOP (COLORGREENTOP+COLORGREENHEIGHT+1)

#define SLIDER (propid==COLORRED || propid==COLORGREEN || propid==COLORBLUE)
#define NUMSTEPS 15
#define STEPVAL (MAXPOT/NUMSTEPS)
#define HBODY 0x1000
#define RP (&(w->WScreen->RastPort))

extern struct TextAttr MyFont; /* the font to use */
extern UBYTE Back[], Fore[];
extern unsigned char pbuf[];

UBYTE ColorTmp[2][3], color_type;

struct Requester ColorRequester; /* the requester */

SHORT ColorReqxys[] = { /* Requester xy pairs */
	0,0,
	COLORREQWIDTH-1,0,
	COLORREQWIDTH-1,COLORREQHEIGHT-1,
	0,COLORREQHEIGHT-1,
	0,0
};

struct Border ColorReqBorder = { /* border for the requester */
	0,0,				/* left,top */
	COLOR1,COLOR0,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	ColorReqxys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

struct IntuiText ColorBlueText = { /* text for BLUE gadget */
	COLOR1,COLOR0,JAM1,-34,1,&MyFont,"B:  ",NULL};

struct Image ColorBlueImage = { /* image for BLUE knob */
	0,0,	/* left, top */
	COLORBLUEWIDTH/(NUMSTEPS+1),COLORBLUEHEIGHT-4,1, /* width, height, depth */
	NULL,0,COLOR1,NULL	/* ImageData, PlanePick, PlaneOnOff, NextImage */
};

struct PropInfo ColorBluePropInfo = { /* prop info for BLUE gadget */
	FREEHORIZ,	/* Flags */
	0,0, 			/* HorizPot, VertPot */
	HBODY,		/* HorizBody */
	0				/* VertBody */
};
	
struct Gadget ColorBlueGadget = { 	/* the BLUE gadget */
	NULL,		/* ptr to next gadget */
	COLORBLUELEFT,COLORBLUETOP,COLORBLUEWIDTH,COLORBLUEHEIGHT,	/* left,top,width,height */
	GADGHNONE,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|GADGIMMEDIATE|FOLLOWMOUSE,	/* Activation flag */
	REQGADGET|PROPGADGET,	/* gadget type */
	&ColorBlueImage,		/* gadget render */
	NULL,			/* select render */
	&ColorBlueText,		/* text for COLORBLUE */
	0,			/* mutual exclude */
	&ColorBluePropInfo,			/* special info */
	COLORBLUE,			/* ID */
	NULL			/* UserData */
};

struct IntuiText ColorGreenText = { /* text for GREEN gadget */
	COLOR1,COLOR0,JAM1,-34,1,&MyFont,"G:  ",NULL};

struct Image ColorGreenImage = { /* image for GREEN knob */
	0,0,	/* left, top */
	COLORGREENWIDTH/(NUMSTEPS+1),COLORGREENHEIGHT-4,1, /* width, height, depth */
	NULL,0,COLOR1,NULL	/* ImageData, PlanePick, PlaneOnOff, NextImage */
};

struct PropInfo ColorGreenPropInfo = { /* prop info for GREEN gadget */
	FREEHORIZ,	/* Flags */
	0,0, 			/* HorizPot, VertPot */
	HBODY,		/* HorizBody */
	0				/* VertBody */
};
	
struct Gadget ColorGreenGadget = { 	/* the GREEN gadget */
	&ColorBlueGadget,		/* ptr to next gadget */
	COLORGREENLEFT,COLORGREENTOP,COLORGREENWIDTH,COLORGREENHEIGHT,	/* left,top,width,height */
	GADGHNONE,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|GADGIMMEDIATE|FOLLOWMOUSE,	/* Activation flag */
	REQGADGET|PROPGADGET,	/* gadget type */
	&ColorGreenImage,		/* gadget render */
	NULL,			/* select render */
	&ColorGreenText,		/* text for COLORGREEN */
	0,			/* mutual exclude */
	&ColorGreenPropInfo,			/* special info */
	COLORGREEN,			/* ID */
	NULL			/* UserData */
};

struct IntuiText ColorRedText = { /* text for RED gadget */
	COLOR1,COLOR0,JAM1,-34,1,&MyFont,"R:  ",NULL};

struct Image ColorRedImage = { /* image for RED knob */
	0,0,	/* left, top */
	COLORREDWIDTH/(NUMSTEPS+1),COLORREDHEIGHT-4,1, /* width, height, depth */
	NULL,0,COLOR1,NULL	/* ImageData, PlanePick, PlaneOnOff, NextImage */
};

struct PropInfo ColorRedPropInfo = { /* prop info for RED gadget */
	FREEHORIZ,	/* Flags */
	0,0, 			/* HorizPot, VertPot */
	HBODY,		/* HorizBody */
	0				/* VertBody */
};
	
struct Gadget ColorRedGadget = { 	/* the RED gadget */
	&ColorGreenGadget,		/* ptr to next gadget */
	COLORREDLEFT,COLORREDTOP,COLORREDWIDTH,COLORREDHEIGHT,	/* left,top,width,height */
	GADGHNONE,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|GADGIMMEDIATE|FOLLOWMOUSE,	/* Activation flag */
	REQGADGET|PROPGADGET,	/* gadget type */
	&ColorRedImage,		/* gadget render */
	NULL,			/* select render */
	&ColorRedText,		/* text for COLORRED */
	0,			/* mutual exclude */
	&ColorRedPropInfo,			/* special info */
	COLORRED,			/* ID */
	NULL			/* UserData */
};

char *ColorStr[2] = {"BACKGROUND", "FOREGROUND"};
struct IntuiText ColorBackText = { /* text for BACK gadget */
	COLOR1,COLOR0,JAM2,1,1,&MyFont,NULL,NULL};

SHORT ColorBackxys[] = { /* BACK gadget xy pairs */
	0,0,
	COLORBACKWIDTH+1,0,
	COLORBACKWIDTH+1,COLORBACKHEIGHT+1,
	0,COLORBACKHEIGHT+1,
	0,0
};

struct Border ColorBackBorder = { 	/* border for BACK gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR0,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	ColorBackxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget ColorBackGadget = { 	/* the BACK gadget */
	&ColorRedGadget,		/* ptr to next gadget */
	COLORBACKLEFT,COLORBACKTOP,COLORBACKWIDTH,COLORBACKHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	GADGIMMEDIATE,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&ColorBackBorder,		/* gadget render */
	NULL,			/* select render */
	&ColorBackText,		/* text for COLORBACK */
	0,			/* mutual exclude */
	NULL,			/* special info */
	COLORBACK,			/* ID */
	NULL			/* UserData */
};

struct IntuiText ColorResetText = { /* text for RESET gadget */
	COLOR1,COLOR0,JAM1,5,1,&MyFont,"RESET",NULL};

SHORT ColorResetxys[] = { /* RESET gadget xy pairs */
	0,0,
	COLORRESETWIDTH+1,0,
	COLORRESETWIDTH+1,COLORRESETHEIGHT+1,
	0,COLORRESETHEIGHT+1,
	0,0
};

struct Border ColorResetBorder = { 	/* border for RESET gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR0,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	ColorResetxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget ColorResetGadget = { 	/* the RESET gadget */
	&ColorBackGadget,		/* ptr to next gadget */
	COLORRESETLEFT,COLORRESETTOP,COLORRESETWIDTH,COLORRESETHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&ColorResetBorder,		/* gadget render */
	NULL,			/* select render */
	&ColorResetText,		/* text for COLORRESET */
	0,			/* mutual exclude */
	NULL,			/* special info */
	COLORRESET,			/* ID */
	NULL			/* UserData */
};

struct IntuiText ColorOkText = { /* text for the OK gadget */
	COLOR1,COLOR0,JAM1,0,1,&MyFont,"  OK  ",NULL};

SHORT ColorOkxys[] = { /* OK gadget xy pairs */
	0,0,
	COLOROKWIDTH+1,0,
	COLOROKWIDTH+1,COLOROKHEIGHT+1,
	0,COLOROKHEIGHT+1,
	0,0
};

struct Border ColorOkBorder = { 	/* border for OK gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR0,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	ColorOkxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget ColorOkGadget = { 	/* the OK gadget */
	&ColorResetGadget,		/* ptr to next gadget */
	COLOROKLEFT,COLOROKTOP,COLOROKWIDTH,COLOROKHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&ColorOkBorder,		/* gadget render */
	NULL,			/* select render */
	&ColorOkText,		/* text for COLOROK */
	0,			/* mutual exclude */
	NULL,			/* special info */
	COLOROK,			/* ID */
	NULL			/* UserData */
};

struct IntuiText ColorCancelText = { /* text for CANCEL gadget */
	COLOR1,COLOR0,JAM1,2,1,&MyFont,"CANCEL",NULL};

SHORT ColorCancelxys[] = { /* CANCEL gadget xy pairs */
	0,0,
	COLORCANCELWIDTH+1,0,
	COLORCANCELWIDTH+1,COLORCANCELHEIGHT+1,
	0,COLORCANCELHEIGHT+1,
	0,0
};

struct Border ColorCancelBorder = { 	/* border for CANCEL gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR0,JAM1,		/* front,back,drawmode */
	5,			/* # of xy pairs */
	ColorCancelxys,		/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget ColorCancelGadget = { /* the CANCEL gadget */
	&ColorOkGadget,		/* ptr to next gadget */
	COLORCANCELLEFT,COLORCANCELTOP,COLORCANCELWIDTH,COLORCANCELHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&ColorCancelBorder,		/* gadget render */
	NULL,			/* special render */
	&ColorCancelText,		/* text for COLORCANCEL */
	0,			/* mutual exclude */
	NULL,			/* special info */
	COLORCANCEL,			/* ID */
	NULL			/* UserData */
};

UWORD colorreqtop, colorreqleft;

ColorReq(w)
struct Window *w;
{
extern UWORD scr_width;

struct IntuiMessage *msg;
struct Gadget *address;
UWORD id, propid = COLOROK;
UBYTE i, done=FALSE;

	colorreqleft = ((scr_width-COLORREQWIDTH)/2);
	colorreqtop = (w->Height - COLORREQHEIGHT) / 2; /* calc top of requester */
	InitRequester(&ColorRequester); /* init requester */
	ColorRequester.LeftEdge = colorreqleft;
	ColorRequester.TopEdge = colorreqtop;
	ColorRequester.Width = COLORREQWIDTH;
	ColorRequester.Height = COLORREQHEIGHT;
	ColorRequester.ReqGadget = &ColorCancelGadget;
	ColorRequester.ReqBorder = &ColorReqBorder;
	ColorRequester.ReqText = NULL;
	ColorRequester.BackFill = COLOR0;
	color_type = 0; /* want to show background color first */
	ColorBackText.IText = &ColorStr[color_type][0];
	Request(&ColorRequester,w); /* put it up */
   while (!done) { /* handle gadgets */
      Wait(1 << w->UserPort->mp_SigBit); /* for for intuition */
      while (msg = (struct IntuiMessage *)GetIntuition(w)) { /* get all msgs */
         switch (msg->Class) {
			case REQSET: /* the requester has come up */
				SetDrMd(RP,JAM2);
				SetAPen(RP,COLOR1);
				SetBPen(RP,COLOR0);
				Set_Tmp_Colors(); /* init the tmp color holders */
				Calc_Gadgets(w); /* calc&show initial gadget posns */
				Print_Color_Numbers(w); /* display numbers */
				break;
			case GADGETDOWN:
#ifdef DEBUG
CDPutStr("gadgetdown\n\r");
#endif
            address = (struct Gadget *)msg->IAddress;
				switch (propid = (UWORD)address->GadgetID) {
				case COLORRED: case COLORGREEN: case COLORBLUE:
#ifdef DEBUG
CDPutStr("     RGB\n\r");
#endif
					ModifyIDCMP(w,IDCMP_FLAGS|MOUSEMOVE); /* I want to follow it */
					break;
				case COLORBACK: /* flip between back & fore colors */
					color_type = !color_type; /* flip */
					ColorBackText.IText = &ColorStr[color_type][0];
					Calc_Gadgets(w); /* calc&show new posns */
					Print_Color_Numbers(w); /* print new numbers */
					break;
				default:break;
				}
				break;
			case MOUSEMOVE:
#ifdef DEBUG
CDPutStr("mousemove\n\r");
#endif
				if (SLIDER) {
					if (Calc_Colors(w)) { /* calc new color vals */
						Screen_Colors(color_type,ColorTmp[color_type][0],
							ColorTmp[color_type][1],ColorTmp[color_type][2]);/* */
						Print_Color_Numbers(w); /* display vals */
					}
				}
				break;
         case GADGETUP:
#ifdef DEBUG
CDPutStr("gadgetup\n\r");
#endif
            address = (struct Gadget *)msg->IAddress;
				switch (address->GadgetID) {
				case COLORRED: case COLORGREEN: case COLORBLUE:
#ifdef DEBUG
CDPutStr("     RGB\n\r");
#endif
					if (Calc_Colors(w)) { /* calc new color vals */
						Screen_Colors(color_type,ColorTmp[color_type][0],
							ColorTmp[color_type][1],ColorTmp[color_type][2]);/* */
						Print_Color_Numbers(w); /* display new color vals */
					}
					ModifyIDCMP(w,IDCMP_FLAGS); /* no more mouse moves */
					break;
				case COLOROK: done = TRUE;
					Set_Real_Colors(); /* set new real colors */
					break;
            case COLORCANCEL: done = TRUE;
					Reset_Tmp_Colors(); /* re-init & show tmp colors */
					break;
				case COLORRESET:
					Reset_Tmp_Colors(); /* re-init & show tmp colors */
					Calc_Gadgets(w); /* calc&show new gadget posns */
					Print_Color_Numbers(w); /* display numbers */
					break;
	         default: break;
				}
				break;
			default: break;
			}
         ReplyMsg(msg); /* reply to the msg */
		}
	}
}

Set_Tmp_Colors() /* transfer real colors to the tmp registers */
{
UBYTE i;
	for (i=0; i<3; i++) { /* for rgb */
		ColorTmp[0][i] = Back[i]; /* background */
		ColorTmp[1][i] = Fore[i]; /* foreground */
	}
}

Reset_Tmp_Colors()
{
UBYTE i;
	Set_Tmp_Colors(); /* re-init tmp colors */
	for (i=0; i<2; i++) { /* for back & fore colors */
		Screen_Colors(i,ColorTmp[i][0],ColorTmp[i][1],ColorTmp[i][2]);
	} /* fix colors */
}

Set_Real_Colors() /* transfer tmp colors to the real registers */
{
UBYTE i;
	for (i=0; i<3; i++) { /* for rgb */
		Back[i] = ColorTmp[0][i]; /* background */
		Fore[i] = ColorTmp[1][i]; /* foreground */
	}
}

Print_Color_Numbers(w) /* display colors as ascii digits */
struct Window *w;
{
static char numbers[] = "0123456789";
UBYTE i, color;
	SetDrMd(RP,JAM2);
	for (i=0; i<3; i++) {
		color = ColorTmp[color_type][i];
		Move(RP,colorreqleft+COLORREDLEFT-18,colorreqtop+COLORREDTOP+15+i*10);
		Text(RP,&numbers[color/10],1);
		Move(RP,colorreqleft+COLORREDLEFT-10,colorreqtop+COLORREDTOP+15+i*10);
		Text(RP,&numbers[color%10],1);
	}
}

Calc_Colors(w) /* calculate the color value from the gadgets posn */
struct Window *w;
{
int err, i;
UBYTE color[3];
#ifdef DEBUG
sprintf(pbuf,"Calc_Colors:RPot=%lx, GPot=%lx, BPot=%lx\n\r",
	ColorRedPropInfo.HorizPot,ColorGreenPropInfo.HorizPot,
	ColorBluePropInfo.HorizPot);
CDPutStr(pbuf);
#endif
	for (i=0; i<3; i++) color[i] = ColorTmp[color_type][i]; /* copy old */
	ColorTmp[color_type][0] = ColorRedPropInfo.HorizPot >> 12;
	ColorTmp[color_type][1] = ColorGreenPropInfo.HorizPot >> 12;
	ColorTmp[color_type][2] = ColorBluePropInfo.HorizPot >> 12;
	err = 0; /* flag no change */
	for (i=0; i<3; i++) err |= (color[i] == ColorTmp[color_type][i]);
	return(err); /* return true if numbers changed */
}

Calc_Gadgets(w) /* calculate & show the gadget posn from the colors */
struct Window *w;
{
	ModifyProp(&ColorRedGadget,w,&ColorRequester,FREEHORIZ,
		ColorTmp[color_type][0]*STEPVAL,0,HBODY,0);
	ModifyProp(&ColorGreenGadget,w,&ColorRequester,FREEHORIZ,
		ColorTmp[color_type][1]*STEPVAL,0,HBODY,0);
	ModifyProp(&ColorBlueGadget,w,&ColorRequester,FREEHORIZ,
		ColorTmp[color_type][2]*STEPVAL,0,HBODY,0);
}
