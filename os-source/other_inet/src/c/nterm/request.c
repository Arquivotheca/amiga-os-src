/*****************************************************************************
* Auto string requester
*****************************************************************************/

#include "st.h"
#include "term.h"

#define COLOR0 0
#define COLOR1 1

#define CANCEL 0
#define OK 1
#define FILE 2

#define FILEMAX 32

#define FILELEFT 60
#define FILETOP 16
#define FILEWIDTH (FILEMAX * 8)
#define FILEHEIGHT 9

#define REQWIDTH (FILEWIDTH + 8 * 8)
#define REQHEIGHT 72

#define OKWIDTH 50
#define OKHEIGHT 9
#define OKLEFT 16
#define OKTOP (REQHEIGHT-OKHEIGHT-8)

#define CANCELWIDTH OKWIDTH
#define CANCELHEIGHT OKHEIGHT
#define CANCELLEFT (REQWIDTH-CANCELWIDTH-OKLEFT)
#define CANCELTOP OKTOP

/* extern char view_name[], print_name[]; */
char filebuffer[FILEMAX];	/* string buffer for filename */
char ufilebuffer[FILEMAX];		/* undo string buffer for filename */

struct Requester MyRequest; /* the requester */

SHORT Reqxys[] = { /* Requester xy pairs */
	0,0,
	REQWIDTH-1,0,
	REQWIDTH-1,REQHEIGHT-1,
	0,REQHEIGHT-1,
	0,0
};

struct Border ReqBorder = { /* border for the requester */
	0,0,						/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	Reqxys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

struct IntuiText FileText = { /* text for filename gadget */
	COLOR1,COLOR1,JAM1,-50,0,NULL,NULL,NULL};

SHORT Filexys[] = { /* Filename gadget xy pairs */
	0,0,
	FILEWIDTH+1,0,
	FILEWIDTH+1,FILEHEIGHT,
	0,FILEHEIGHT,
	0,0
};

struct Border FileBorder = { /* border for Filename gadget */
	-1,-1,				/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	5,						/* # of xy pairs */
	Filexys,				/* array of xy pairs */
	NULL					/* ptr to another border */
};

struct StringInfo	filestringinfo = {
	filebuffer,	/* buffer */
	ufilebuffer,	/* undo buffer */
	0,		/* char posn in buffer */
	FILEMAX,	/* max # of chars */
	};

struct Gadget FileGadget = { 	/* the filename gadget */
	NULL,			/* ptr to next gadget */
	FILELEFT,FILETOP,FILEWIDTH,FILEHEIGHT,	/* left,top,width,height */
	GADGHCOMP,	/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,		/* Activation Flag */
	REQGADGET|STRGADGET,	/* gadget type */
	&FileBorder,		/* gadget render */
	NULL,			/* select render */
	&FileText,		/* text for filename prompt */
	0,			/* mutual exclude */
	&filestringinfo,	/* special info */
	FILE,			/* ID */
	NULL			/* UserData */
};

struct IntuiText OkText = { /* text for OK gadget */
	COLOR1,COLOR1,JAM1,0,1,NULL,"  OK  ",NULL};

SHORT Okxys[] = { /* OK gadget xy pairs */
	0,0,
	OKWIDTH+1,0,
	OKWIDTH+1,OKHEIGHT+1,
	0,OKHEIGHT+1,
	0,-1,
	OKWIDTH+2,-1,
	OKWIDTH+2,OKHEIGHT+2,
	-1,OKHEIGHT+2,
	-1,-1
};

struct Border OkBorder = { 	/* border for OK gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,	/* front,back,drawmode */
	9,			/* # of xy pairs */
	Okxys,			/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget OkGadget = { 	/* the OK gadget */
	&FileGadget,		/* ptr to next gadget */
	OKLEFT,OKTOP,OKWIDTH,OKHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&OkBorder,		/* gadget render */
	NULL,			/* select render */
	&OkText,		/* text for OK */
	0,			/* mutual exclude */
	NULL,			/* special info */
	OK,			/* ID */
	NULL			/* UserData */
};

struct IntuiText CancelText = { /* text for CANCEL gadget */
	COLOR1,COLOR1,JAM1,1,1,NULL,"CANCEL",NULL};

SHORT Cancelxys[] = { /* CANCEL gadget xy pairs */
	0,0,
	CANCELWIDTH+1,0,
	CANCELWIDTH+1,CANCELHEIGHT+1,
	0,CANCELHEIGHT+1,
	0,0
};

struct Border CancelBorder = { 	/* border for CANCEL gadget */
	-1,-1,			/* left,top */
	COLOR1,COLOR1,JAM1,		/* front,back,drawmode */
	5,			/* # of xy pairs */
	Cancelxys,		/* array of xy pairs */
	NULL			/* ptr to another border */
};

struct Gadget CancelGadget = { /* the CANCEL gadget */
	&OkGadget,		/* ptr to next gadget */
	CANCELLEFT,CANCELTOP,CANCELWIDTH,CANCELHEIGHT,	/* left,top,width,height */
	GADGHCOMP,		/* Gad_Flags (what to do when selected) */
	RELVERIFY|ENDGADGET,	/* Activation flag */
	REQGADGET|BOOLGADGET,	/* gadget type */
	&CancelBorder,		/* gadget render */
	NULL,			/* special render */
	&CancelText,		/* text for CANCEL */
	0,			/* mutual exclude */
	NULL,			/* special info */
	CANCEL,			/* ID */
	NULL			/* UserData */
};

struct IntuiText Req2Msg = { /* message3 (the title) for the requester */
	COLOR1,COLOR1,JAM1,0,4,NULL,NULL,NULL};

struct IntuiText ReqMsg = { /* message for the requester */
	COLOR1,COLOR1,JAM1,OKLEFT,OKTOP-20,NULL,NULL,&Req2Msg};


StringRequest(w,s,title)
struct Window *w;
char *s, *title;
{
	SuperStringRequest(w,s,title,"Name:","Enter name and press RETURN");
}


UnitRequest(w,s,title)
struct Window *w;
char *s, *title;
{
	SuperStringRequest(w,s,title,"Unit:","Enter unit and press RETURN");
}



SuperStringRequest(w,s,title,side,bottom) /* string requester */
struct Window *w;
char *s, *title;
char *side;
char *bottom;
{
extern UWORD scr_width;

struct IntuiMessage *msg; /* ptr to the messages */
struct Gadget *address;			/* the message address */
USHORT id;				/* the message id */
UBYTE done = FALSE;
UWORD reqtop;
UWORD REQLEFT = ((scr_width-REQWIDTH)/2);

	reqtop = (w->Height - REQHEIGHT) / 2;
	InitRequester(&MyRequest); /* init reqestor */
	MyRequest.LeftEdge = REQLEFT;
	MyRequest.TopEdge = reqtop;
	MyRequest.Width = REQWIDTH;
	MyRequest.Height = REQHEIGHT;
	MyRequest.ReqGadget = &CancelGadget;
	MyRequest.ReqBorder = &ReqBorder;
	MyRequest.ReqText = &ReqMsg;
	MyRequest.BackFill = COLOR0;
	strcpy(filebuffer,s); /* copy it into the buffer */
	filestringinfo.BufferPos = strlen(s); /* set the default string posn */
	filestringinfo.MaxChars = FILEMAX; 
	filestringinfo.DispPos = 0;
	filestringinfo.UndoPos = FILEMAX;
	filestringinfo.NumChars = 0;
	filestringinfo.DispCount = 0;
	ReqMsg.LeftEdge = (REQWIDTH-strlen(bottom)*8)/2; /* center msg */
	Req2Msg.LeftEdge = (REQWIDTH-strlen(title)*8)/2; /* center title */
	Req2Msg.IText   = title;
	ReqMsg.IText    = bottom;
	FileText.IText  = side;
	Request(&MyRequest,w); /* put up the requester */

	while (!done) { /* handle gadgets */
		Wait(1 << w->UserPort->mp_SigBit); /* for for intuition */
	   while (msg = (struct IntuiMessage *)GetIntuition(w)) { /* get all msgs */
	      switch (msg->Class) {
	      case REQSET:
		 ActivateGadget(&FileGadget,w,&MyRequest);
		 break;
	      case GADGETUP:
	         address = (struct Gadget *)msg->IAddress;
	         id = address->GadgetID;
     		   if (id==OK || id==FILE) {
					/* assign string new value */
					strcpy(s,filebuffer);

					/* give name to the view file */
					/* strcpy(view_name,filebuffer); */
					/* give name to the print file */
					/* strcpy(print_name,filebuffer); */
					done = TRUE;
				}
				if (id==CANCEL) done = TRUE;
				break;
	      default: break;
			}
	      ReplyMsg(msg); /* reply to the msg */
		}
	}
	return(id==OK || id==FILE); /* return status */
}

