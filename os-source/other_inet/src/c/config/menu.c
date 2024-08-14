/* ---------------------------------------------------------------------------------
 * menu.c  (CONFIG)
 *
 * $Locker:  $
 *
 * $Id: menu.c,v 1.1 90/11/07 12:16:02 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: Hog:Other/inet/src/c/config/RCS/menu.c,v 1.1 90/11/07 12:16:02 bj Exp $
 *
 *-----------------------------------------------------------------------------------
 */

/*
 * Graphical configurator
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <pwd.h>
#include <bstr.h>

#ifdef LATTICE
#include <proto/all.h>
#endif

#include <config.h>

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#define WIDTH 		nw.Width
#define HEIGHT 		nw.Height
#define VPITCH		(fontheight + 4)
#define FIELDWIDTH	17
#define TOPOFFSET	5

#define NUM_PAIR 5
static struct {
	short	x, y;
} bord[NUM_PAIR] = {{-1, -1}, {0, -1}, {0, 0}, {-1, 0}, {-1, -1}};

#define INTUITEXT(p) {1, 2, JAM1, 0, 0, NULL, (UBYTE *) p, NULL}

static struct IntuiText text[] = {
#define VAL_HOSTNAME 	0	
	INTUITEXT("Hostname"),
#define VAL_USERNAME 	1	
	INTUITEXT("Username"),
#define VAL_UMASK 	2	
	INTUITEXT("NFS Mask"),
#define VAL_TZ		3	
	INTUITEXT("Timezone"),
#define VAL_GATEWAY	4	
	INTUITEXT("Gateway addr"),
#define VAL_SUBNET_MASK 5	
	INTUITEXT("Subnet mask"),
#define VAL_BRDCAST	6	
	INTUITEXT("Broadcast addr"),
/*
#define VAL_GID		7	
	INTUITEXT("Groups"),
#define VAL_FILE	8	
	INTUITEXT("Config File")
*/
};

#define NUMITEXT (sizeof(text)/sizeof(text[0]))
static char cycler[NUMITEXT];
static struct Border strb = {0, 0, 1, 0, JAM1, NUM_PAIR, (short *)bord, NULL};
static struct StringInfo strings[NUMITEXT];
static struct Gadget gadgets[NUMITEXT];
static struct Window *display;
static char valbuf[NUMITEXT][80];
static char undo_buf[2*sizeof(valbuf[0])];
static fontwidth, fontheight;
static textoffset = -1;
static struct NewWindow nw = {
   	160, 50, 0, 0, -1, -1,
   	CLOSEWINDOW | REFRESHWINDOW | MOUSEBUTTONS | GADGETDOWN	| GADGETUP,
 	WINDOWDRAG | WINDOWCLOSE | SMART_REFRESH, NULL, NULL,
   	(UBYTE *)" Network Configuration ", NULL, NULL,	0,0,0,0, WBENCHSCREEN,
};
static struct {
	short	offset;
	char	name[4];
} tz[] = {
	{240, "EDT"}, {300, "CDT"}, {360, "MDT"}, {420, "PDT"},
	{300, "EST"}, {360, "CST"}, {420, "MST"}, {480, "PST"}, 
	{-1, 0L}
};

void close_Screen(), main_Loop(), screen_up();

static void
screen_up()
{
	int top, left, i, scrwidth, scrheight;
	register struct Gadget *p;
	struct Screen scr;
	
	if(!(GfxBase = (void *)OpenLibrary("graphics.library",0L))){
       		close_Screen();
    	}
  
  	if(!(IntuitionBase = (void *)OpenLibrary("intuition.library",0L))){
       		close_Screen();
    	}
  
	scrwidth = 640; scrheight = 200;
	if(GetScreenData(&scr, sizeof(struct Screen), WBENCHSCREEN, NULL)){
		fontheight = scr.RastPort.Font->tf_YSize;
		fontwidth = scr.RastPort.Font->tf_XSize;
		scrwidth = scr.Width;
		scrheight = scr.Height;
	}

	for(i = 0; i < NUMITEXT; i++){
		int len;

		if((len = strlen(text[i].IText)) > textoffset){
			textoffset = len;
		}
	}

    	textoffset += 2;
	bord[1].x = FIELDWIDTH*fontwidth;
	bord[2].x = FIELDWIDTH*fontwidth; bord[2].y = fontheight;
	bord[3].y = fontheight;
	nw.Flags |= WINDOWCLOSE;
	nw.Title = (UBYTE *)" Network Configuration ";
	WIDTH = fontwidth*(textoffset + FIELDWIDTH) + 14;
	HEIGHT = VPITCH*NUMITEXT + 16;
	nw.TopEdge = (scrheight - HEIGHT)/2;
	nw.LeftEdge = (scrwidth - WIDTH)/2;

    	if(!(display = (struct Window *)OpenWindow(&nw) )) {
       		close_Screen();
    	}

	SetDrMd(display->RPort, JAM1);
	SetAPen(display->RPort, 2); 
	RectFill(display->RPort, 
	    		display->BorderLeft-1, 
			display->BorderTop-1,
			WIDTH - display->BorderRight, 
			HEIGHT-display->BorderBottom);

	top = display->BorderTop + TOPOFFSET; 
	left = display->BorderLeft + 2;
	for(i = 0; i < NUMITEXT; i++){
		strings[i].Buffer = (UBYTE *)valbuf[i];
		strings[i].UndoBuffer = (UBYTE *)undo_buf;
		strings[i].MaxChars = sizeof(valbuf[i])-2;
		p = &gadgets[i];
		p->LeftEdge = left + fontwidth*textoffset;
		p->TopEdge = top;
		p->Width = fontwidth*FIELDWIDTH;
		p->Height = fontheight;
		if(cycler[i]){
			p->Activation = RELVERIFY|GADGIMMEDIATE;
			p->GadgetType = BOOLGADGET;
			p->Flags = GADGHNONE;
		} else {
			p->Flags = GADGHCOMP;
			p->Activation = RELVERIFY|GADGIMMEDIATE|STRINGCENTER;
			p->GadgetType = STRGADGET;
		}
		p->GadgetRender = (APTR)&strb;
		p->GadgetText = &text[i];
		p->GadgetID = i;
		p->SpecialInfo = (APTR)&strings[i];
		p->GadgetText->LeftEdge = -textoffset*fontwidth;
		p->GadgetText->TopEdge = 0;

		top += VPITCH;

		AddGadget(display, p, -1);
	}

	RefreshGadgets(display->FirstGadget, display, NULL);
}

/*
 * Main loop of program.  This this is driven by two sources of
 * events: ethernet packet reception, and window events caused by
 * user.
 */

/* I rewrote this because it really didn't do anything.  MMH */

static void
main_Loop()
{
	ULONG MessageClass;
	struct IntuiMessage *message;

	for (;;){
		Wait(1L << display->UserPort->mp_SigBit);
		while (message = (struct IntuiMessage *)GetMsg(display->UserPort)) {
			MessageClass = message->Class;
			ReplyMsg((struct Message *)message);
			if (MessageClass==CLOSEWINDOW)
				return;
		}
	}

}

static void 
close_Screen()
{
    	if(display){
		CloseWindow(display);
	}

    	if(GfxBase){
		CloseLibrary(GfxBase);
	}

    	if(IntuitionBase){
		CloseLibrary(IntuitionBase);
	}
}

int  
configmenu(cf)
	register struct config *cf;
{
	static struct config co;
	struct passwd *pw, *getpwnam();
	register int i;

	if(!cf){
		return 0;
	}

	co = *cf;
	strcpy(valbuf[VAL_TZ], cf->tzname);
	strcpy(valbuf[VAL_HOSTNAME], cf->host);
	strcpy(valbuf[VAL_USERNAME], cf->username);
	strcpy(valbuf[VAL_GATEWAY], cf->gateway);
	strcpy(valbuf[VAL_SUBNET_MASK], cf->subnetmask);
	strcpy(valbuf[VAL_BRDCAST], cf->broadcast);
	sprintf(valbuf[VAL_UMASK], "%03o", cf->umask);
/*	strcpy(valbuf[VAL_FILE], cf->configfile); */

	screen_up();
	main_Loop();
	close_Screen();

	strcpy(cf->host, valbuf[VAL_HOSTNAME]);
	strcpy(cf->username, valbuf[VAL_USERNAME]);
	cf->gid = cf->uid = -2;
	pw = getpwnam(cf->username);
	if(pw){
		cf->uid = pw->pw_uid;
		cf->gid = pw->pw_gid;
	}
	valbuf[VAL_UMASK][3]='\0';
	cf->umask =(short)atoi(&valbuf[VAL_UMASK][2]);
	valbuf[VAL_UMASK][2]='\0';
	cf->umask+=(short)atoi(&valbuf[VAL_UMASK][1])*8;
	valbuf[VAL_UMASK][1]='\0';
	cf->umask+=(short)atoi(valbuf[VAL_UMASK])*64;

	for(i = 0; tz[i].name; i++){
		if(strcasecmp(valbuf[VAL_TZ], tz[i].name) == 0){
			break;
		}
	}
	cf->tz_offset = tz[i].name? tz[i].offset:atoi(valbuf[VAL_TZ]);

	strcpy(cf->tzname, valbuf[VAL_TZ]);
	strcpy(cf->gateway, valbuf[VAL_GATEWAY]);
	strcpy(cf->subnetmask, valbuf[VAL_SUBNET_MASK]);
	strcpy(cf->broadcast, valbuf[VAL_BRDCAST]);
/*	strcpy(cf->configfile, valbuf[VAL_FILE]); */

	return (bcmp(&co, cf, sizeof(co)) != 0);
}
