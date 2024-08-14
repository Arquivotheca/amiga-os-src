/*****************************************************************************
* Meta requester
*****************************************************************************/

#include "st.h"
#include "meta.h"
#include "term.h"

#define COLOR0 0
#define COLOR1 1
#define FSPACING 12

#define CANCELM 0
#define OKM 1
#define F1 2
#define F2 3
#define F3 4
#define F4 5
#define F5 6
#define F6 7
#define F7 8
#define F8 9
#define F9 10
#define F10 11

#define METAHEIGHT (40 + NUMFUNC * FSPACING)

#define FWIDTH (FLEN*8)
#define FHEIGHT 9
#define FTOP 18

#define OKMWIDTH 50
#define OKMHEIGHT 9
#define OKMLEFT 16
#define OKMTOP (METAHEIGHT-OKMHEIGHT-8)

#define CANCELMWIDTH 50
#define CANCELMHEIGHT 9
#define CANCELMTOP (OKMTOP)

extern char fkeys[FMAX][FLEN+2]; /* the current function key strings */

char fbuffer[NUMFUNC][FLEN]; /* string buffer for the func key defns */
char ufbuffer[NUMFUNC][FLEN]; /* undo buffer for the above */

struct Requester MetaRequester; /* the requester */

SHORT Metaxys[] = { /* Requester xy pairs */
	0,0,
	0,0,
	0,METAHEIGHT-1,
	0,METAHEIGHT-1,
	0,0
};

struct Border MetaBorder = { /* border for the requester */
	0,0,						/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	Metaxys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

struct IntuiText FText[NUMFUNC] = { /* text for func gadgets */
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F01:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F02:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F03:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F04:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F05:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F06:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F07:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F08:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F09:",NULL},
	{COLOR1,COLOR1,JAM1,(-4*10),0,NULL,"F10:",NULL},
};

SHORT Fxys[] = { /* Func gadget xy pairs */
	0,0,
	FWIDTH+1,0,
	FWIDTH+1,FHEIGHT,
	0,FHEIGHT,
	0,0
};

struct Border FBorder = { /* border for func gadget */
	-1,-1,				/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	Fxys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

struct StringInfo	fstringinfo[NUMFUNC] = {
	{&fbuffer[0][0],	/* buffer */
	&ufbuffer[0][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[1][0],	/* buffer */
	&ufbuffer[1][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[2][0],	/* buffer */
	&ufbuffer[2][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[3][0],	/* buffer */
	&ufbuffer[3][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[4][0],	/* buffer */
	&ufbuffer[4][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[5][0],	/* buffer */
	&ufbuffer[5][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[6][0],	/* buffer */
	&ufbuffer[6][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[7][0],	/* buffer */
	&ufbuffer[7][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[8][0],	/* buffer */
	&ufbuffer[8][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	},
	{&fbuffer[9][0],	/* buffer */
	&ufbuffer[9][0],	/* undo buffer */
	0,		/* char posn in buffer */
	FLEN,	/* max # of chars */
	}
};

struct Gadget FGadget[NUMFUNC] = { 	/* the func gadgets */
	{&FGadget[1],			/* ptr to next gadget */
	0,FTOP,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[0],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[0],	/* special info */
	F1,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[2],			/* ptr to next gadget */
	0,FTOP+16,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[1],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[1],	/* special info */
	F2,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[3],			/* ptr to next gadget */
	0,FTOP+32,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[2],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[2],	/* special info */
	F3,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[4],			/* ptr to next gadget */
	0,FTOP+48,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[3],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[3],	/* special info */
	F4,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[5],			/* ptr to next gadget */
	0,FTOP+64,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[4],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[4],	/* special info */
	F5,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[6],			/* ptr to next gadget */
	0,FTOP+80,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[5],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[5],	/* special info */
	F6,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[7],			/* ptr to next gadget */
	0,FTOP+96,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[6],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[6],	/* special info */
	F7,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[8],			/* ptr to next gadget */
	0,FTOP+112,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[7],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[7],	/* special info */
	F8,			/* ID */
	NULL			/* UserData */
	},
	{&FGadget[9],			/* ptr to next gadget */
	0,FTOP+128,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[8],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[8],	/* special info */
	F9,			/* ID */
	NULL			/* UserData */
	},
	{NULL,			/* ptr to next gadget */
	0,FTOP+144,FWIDTH,FHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FBorder,		/* gadget render */
	NULL,			/* select render */
	&FText[9],		/* text for f1name prompt */
	0,			/* mutual exclude */
	&fstringinfo[9],	/* special info */
	F10,			/* ID */
	NULL			/* UserData */
	}
};

struct IntuiText OkmText = { /* text for OKM gadget */
	COLOR1,COLOR1,JAM1,0,1,NULL,"  OK  ",NULL};

SHORT Okmxys[] = { /* OKM gadget xy pairs */
	0,0,
	OKMWIDTH+1,0,
	OKMWIDTH+1,OKMHEIGHT+1,
	0,OKMHEIGHT+1,
	0,0
};

struct Border OkmBorder = { 	/* border for OKM gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,			/* # of xy pairs */
	Okmxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget OkmGadget = { 	/* the OKM gadget */
	&FGadget[0],		/* ptr to next gadget */
	OKMLEFT,OKMTOP,OKMWIDTH,OKMHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&OkmBorder,		/* gadget render */
	NULL,			/* select render */
	&OkmText,		/* text for OKM */
	0,			/* mutual exclude */
	NULL,			/* special info */
	OKM,			/* ID */
	NULL			/* UserData */
};

struct IntuiText CancelmText = { /* text for CANCELM gadget */
	COLOR1,COLOR1,JAM1,1,1,NULL,"CANCEL",NULL};

SHORT Cancelmxys[] = { /* CANCELM gadget xy pairs */
	0,0,
	CANCELMWIDTH+1,0,
	CANCELMWIDTH+1,CANCELMHEIGHT+1,
	0,CANCELMHEIGHT+1,
	0,0
};

struct Border CancelmBorder = { 	/* border for CANCELM gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,		/* front,back,drawmode */
	5,			/* # of xy pairs */
	Cancelmxys,		/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget CancelmGadget = { /* the CANCELM gadget */
	&OkmGadget,		/* ptr to next gadget */
	0,CANCELMTOP,CANCELMWIDTH,CANCELMHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&CancelmBorder,		/* gadget render */
	NULL,			/* special render */
	&CancelmText,		/* text for CANCELM */
	0,			/* mutual exclude */
	NULL,			/* special info */
	CANCELM,			/* ID */
	NULL			/* UserData */
};

char MetaTitle[] = "Function Key Strings";
struct IntuiText MetaMsg2 = { /* message for the requester */
	COLOR1,COLOR1,JAM1,0,4,NULL,MetaTitle,NULL};

char MetaText[] = "Enter string and press RETURN.";
struct IntuiText MetaMsg = { /* message for the requester */
	COLOR1,COLOR1,JAM1,0,OKMTOP,NULL,MetaText,&MetaMsg2};

MetaRequest(w) /* meta requester */
struct Window *w;
{
extern UWORD scr_width;

struct IntuiMessage *msg; /* ptr to the messages */
struct Gadget *address;			/* the message address */
USHORT id;				/* the message id */
UBYTE done = FALSE, i;
UWORD metatop;
UWORD METAWIDTH = scr_width;
UWORD METALEFT = ((scr_width-METAWIDTH)/2);
UWORD FLEFT = ((METAWIDTH-FWIDTH)/2);
UWORD CANCELMLEFT = (METAWIDTH-CANCELMWIDTH-16);

	metatop = (w->Height - METAHEIGHT) / 2;
	InitRequester(&MetaRequester); /* init reqestor */
	MetaRequester.LeftEdge = METALEFT;
	MetaRequester.TopEdge = metatop;
	MetaRequester.Width = METAWIDTH;
	MetaRequester.Height = METAHEIGHT;
	MetaRequester.ReqGadget = &CancelmGadget;
	MetaRequester.ReqBorder = &MetaBorder;
	MetaRequester.ReqText = &MetaMsg;
	MetaRequester.BackFill = COLOR0;
	MetaMsg.LeftEdge = (METAWIDTH - strlen(MetaText) * 8 - 8) / 2; /* center */
	MetaMsg2.LeftEdge = (METAWIDTH - strlen(MetaTitle) * 8 - 8) / 2; /* ditto */
	for (i=0; i<NUMFUNC; i++) {
		strcpy(&fbuffer[i][0],&fkeys[i][2]); /* copy string defns */
		fstringinfo[i].BufferPos = strlen(&fbuffer[i][0]); /* set length */
		FGadget[i].TopEdge = FTOP + i * FSPACING;
		FGadget[i].LeftEdge = FLEFT;
	}
	Metaxys[2] = METAWIDTH - 1;
	Metaxys[4] = METAWIDTH - 1;
	CancelmGadget.LeftEdge = CANCELMLEFT;
	Request(&MetaRequester,w); /* put up the requester */

	while (!done) { /* handle gadgets */
		Wait(1 << w->UserPort->mp_SigBit); /* for for intuition */
	   while (msg = (struct IntuiMessage *)GetIntuition(w)) { /* get all msgs */
	      switch (msg->Class) {
	      case GADGETUP:
	         address = (struct Gadget *)msg->IAddress;
	         switch(id = address->GadgetID) {
     		   case OKM: done = TRUE;
					for (i=0; i<NUMFUNC; i++) {
						Set_Function_Key(&fbuffer[i][0],i+1); /* set new defns */
/*						strcpy(&fkeys[i][2],&fbuffer[i][0]); /* set new strng defns */
/*						fkeys[i][0] = strlen(&fbuffer[i][0]); /* set new length */
					}
					break;
				case CANCELM: done = TRUE;
					break;
		      default: break; /* do nothing for F1 to F10 */
				}
			}
	      ReplyMsg(msg); /* reply to the msg */
		}
	}
	return(id==OKM); /* return status */
}
