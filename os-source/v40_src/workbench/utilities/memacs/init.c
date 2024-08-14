#include "ed.h"

WORD	*BindKey;
WORD	*TBindKey;

#if	AMIGA
#include "workbench/startup.h"
#include "memacs_rev.h"

#define IS_PAL (((struct GfxBase *)GfxBase)->DisplayFlags & PAL)

#define BLUE 0
#define WHITE 1

struct MinList paste;
struct MinNode *pastenode;
LONG pcount;

UBYTE screenname[30]="MEMACS_01";
UBYTE *PointerData;
UBYTE *PrefBuffer;
UBYTE *lineBuffer;
int linecounter;
int LaceFlag;
int fullsize;
UBYTE *fkm[35];
UBYTE *fileBuffer;
UBYTE *copybuffer;
struct FileInfoBlock *finfo;
struct TextFont *tf;
BOOL oflag;


extern UBYTE *fkmdef[35];
extern int gotoLine;
extern LONG opts[OPT_COUNT];
extern int version;
extern struct RDargs *rdargs;
extern struct MenuItem edititems[];
extern struct WBStartup *WBenchMsg;
extern struct Library *OpenLibrary();
extern struct MsgPort consoleMsgPort;
extern struct Menu menus[];
extern int RememberFile;

extern EWINDOW *wheadp;   /* EWINDOW listhead */

struct Screen *OpenScreen();
struct Window *OpenWindow();
struct Screen *screen;
struct Window *window;

extern LONG  OrigDir;
extern struct WBArg *Arg;

struct IntuiMessage *message;
struct Remember *RememberKey;

struct Library *IntuitionBase=NULL;
struct Library *DOSBase=NULL;
struct Library *GfxBase=NULL;
struct Library *IFFParseBase=NULL;

struct ExtNewScreen ns = {
	0,0,
	640,200,1, /* & depth */
	BLUE,WHITE,
	HIRES,	/* viewmodes */
	CUSTOMSCREEN|NS_EXTENDED,
	NULL, /* default font */
	"MicroEMACS V2.1",
	NULL, /* no user gadgets */
	NULL, /* default bit map */
	NULL}; /* for extension tags */

struct NewWindow nw = {
        0, 10,        	/* starting position (left,top) */
	640,190,        /* width, height */
        BLUE,WHITE,    /* detailpen, blockpen */
        MENUPICK|MOUSEBUTTONS|INACTIVEWINDOW,	/* flags for idcmp */
        SMART_REFRESH|ACTIVATE|BORDERLESS, /* window gadget flags */
        NULL,           /* pointer to 1st user gadget */
        NULL,           /* pointer to user check */
	NULL,		 /* no title */
        NULL,           /* pointer to window screen (add after it is open */
        NULL,           /* pointer to super bitmap */
        100,50,         /* min width, height */
        ~0,~0,        /* max width, height */
        CUSTOMSCREEN};

void aStart(argc,argv)
int argc;
UBYTE **argv;
{
int i;

screen=NULL;
window=NULL;
IntuitionBase=NULL;
GfxBase=NULL;
RememberKey=NULL;
fileBuffer=NULL;
PointerData=NULL;
PrefBuffer=NULL;
lineBuffer=NULL;
BindKey=NULL;
TBindKey=NULL;
finfo=NULL;
tf=NULL;
oflag=FALSE;
fullsize=FALSE;
copybuffer=NULL;
pcount=0;
pastenode=0;
rdargs=NULL;
NewList(&paste);
RememberFile=0;

if((IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",33))== NULL)
	errorExit(10);
if((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)) ==NULL)
	errorExit(10);

if(argc) {
    if(version >= 36) {
        if(! (rdargs = ReadArgs(TEMPLATE,opts,NULL))) {
	    PrintFault(IoErr(),NULL);
	    errorExit(10);
	}

	if(opts[OPT_OPT]) {
	    if (( ((UBYTE *)opts[OPT_OPT])[0]&0x5F) == 'F')fullsize=TRUE;
	        SetForWindow();
	}
        /* check for window full size specifiers */
        if(opts[OPT_GOTO]) {
	    if((gotoLine=atoi((UBYTE *)opts[OPT_GOTO])) > 0) gotoLine--;
	}
	/* check if conclip is running.  If not, turn off paste menu item */
	if(!FindPort("ConClip.rendezvous")) {
		/* maybe zapping the command byte would be better ? */
		/* this turns the menuitem off */
		edititems[16].NextItem = NULL;
	}
    }
  else {
	/* limited pre V36 argument code */
	if(((argc==3) && !stricmp(argv[1],"OPT")) ||
	 ((argc > 3) && !stricmp(argv[2],"OPT"))) {
	    if(!stricmp(argv[argc-1],"F"))fullsize=TRUE;
	    SetForWindow();
  	}
  }
}

if(!(fileBuffer = (UBYTE *)
    AllocRemember(&RememberKey,MAXFILEBUF+4,MEMF_PUBLIC|MEMF_CLEAR)))
	errorExit(10);
if(!(PointerData=(UBYTE *)AllocRemember(&RememberKey,128,MEMF_CHIP|MEMF_CLEAR)))
     errorExit(10);
if(!(PrefBuffer=(UBYTE *)AllocRemember(&RememberKey,128,MEMF_CHIP|MEMF_CLEAR)))
    errorExit(10);
if(!(lineBuffer=(UBYTE *)
    AllocRemember(&RememberKey,MAXLINE+1,MEMF_PUBLIC|MEMF_CLEAR)))
	errorExit(10);
if(!(BindKey=(WORD *)
    AllocRemember(&RememberKey,NBINDW,MEMF_PUBLIC|MEMF_CLEAR)))errorExit(10);
if(!(TBindKey=(WORD *)
    AllocRemember(&RememberKey,NBINDW,MEMF_PUBLIC|MEMF_CLEAR)))errorExit(10);

if(!(finfo=(struct FileInfoBlock *)
	AllocRemember(&RememberKey,sizeof(struct FileInfoBlock),
	MEMF_PUBLIC|MEMF_CLEAR)))errorExit(10);

if(!(copybuffer=(UBYTE *)AllocMem(2048,MEMF_PUBLIC|MEMF_CLEAR)))errorExit(10);

OrigDir=NULL;
linecounter=0;

for(i=0; i<35; i++) {
    if(!(fkm[i] = (UBYTE *)AllocRemember(&RememberKey,80,MEMF_PUBLIC|MEMF_CLEAR)))
	errorExit(10);
}

InitFunctionKeys(0,0);
LaceFlag=GetWindow(-1); /* So, how big is it, Sir ? */
}




struct Rectangle rect = {0,0,0,0};

UWORD mypens[]={~0};
struct TagItem tags[] = {
	{SA_PubName,NULL},
	{SA_DisplayID,NULL},	
	{SA_Pens,(ULONG)mypens},
	{TAG_DONE,}
};

int GetWindow(flag)
int flag;
{

USHORT height,width;
UBYTE *sbuffer;
struct Process *process;
struct Screen *pscreen;
int localLace,maxh=200;
int i=1;
LONG id;

if(IS_PAL) maxh=256;

if(!(sbuffer=(UBYTE *)AllocMem(sizeof(struct Screen),MEMF_PUBLIC)))
	errorExit(10);
GetScreenData(sbuffer, sizeof(struct Screen), WBENCHSCREEN, NULL);

localLace= ((struct Screen *)sbuffer)->ViewPort.Modes | HIRES;
height=(SHORT)((struct Screen *)sbuffer)->Height;
width=(SHORT)((struct Screen *)sbuffer)->Width;
FreeMem(sbuffer,sizeof(struct Screen));

if(version >= 36) {
    pscreen=LockPubScreen(NULL); /* go to default */
    if(pscreen) {

        id=GetVPModeID(&pscreen->ViewPort);
	/* use old style lace bit */
/*        localLace= pscreen->ViewPort.Modes | HIRES; */

        if(id != INVALID_ID)QueryOverscan(id,&rect,OSCAN_TEXT);
	UnlockPubScreen(NULL,pscreen);
    }
    height=rect.MaxY-rect.MinY+1;
    width=rect.MaxX-rect.MinX+1;
}

nw.Width=ns.Width=width;
if (flag == -1) { /* Initial size calculations */
		  /* figure out LACE, but we were told the height */
    ns.ViewModes = localLace;	/* match their WB viewmodes */
}
if(localLace & LACE) maxh *= 2; /* pick proper max height */

if(nw.Type != WBENCHSCREEN) {   /* on a custom screen */
    /* only allow them to switch lace modes on custom screen */
    if(!flag) { /* we want to leave interlace */
        if (localLace & LACE) height /= 2;
    }
    else if(flag > 0) { /* we are trying to enter interlace */
        if(!(localLace & LACE)) { /* if not already in interlace */ 
	    height *= 2;
	    maxh *= 2;
	}
    }
	/* now entering the Hedley Zone */
    if (height >= 800 ) {
	tags[1].ti_Tag = TAG_IGNORE; /* let's not open another hedley */
	height = maxh; /* hedley screen kluge */
	if(width > 1000) {
	    nw.Width=ns.Width=640;
    	}
    }
    ns.Height=height;

    ns.Extension=tags;
    if(version >= 36) {
	/* find an unused name slot */
	while(screen=LockPubScreen(screenname)) {
	    UnlockPubScreen(screenname,NULL);
	    itoa(&screenname[6],2,++i);
	    screenname[6]='_';
	}
        tags[0].ti_Data=(ULONG)screenname;
	if(flag==1)id |= (USHORT)INTERLACE;
        tags[1].ti_Data=(ULONG)id;
    }

    if((screen=OpenScreen(&ns)) == NULL)errorExit(10);
    if(version >= 36) {
	PubScreenStatus(screen,0);
    }
    nw.TopEdge=screen->BarHeight;
    nw.Height=(SHORT)height-screen->BarHeight; /*-(screen->BarHeight/2); */
    nw.Screen=screen;
}

else {	/* take what they have the workbench set to (almost) */
    nw.Height=height;
    nw.Screen=screen=NULL;
    /* don''t open full screen unless they specifically request it */
    if(!fullsize & (height>maxh)) {
	if((height/2)>maxh) nw.Height=height/2;
	else nw.Height=maxh;
	if(width >= 656) nw.Width=656;
    }
}

if((window=OpenWindow(&nw)) == NULL)errorExit(10); 

SetDrMd(window->RPort,JAM2);
SetAPen(window->RPort,WHITE);
SetBPen(window->RPort,BLUE);

SetMenuStrip(window,&menus[0]);
SetMyPointer();
process = (struct Process *)FindTask(NULL); /* set error window */
process->pr_WindowPtr=(APTR)window;

GetWindowSize();

if(localLace&LACE)return(TRUE);
return(FALSE);
}

void GetWindowSize()
{
long awidth,aheight;

     if(!(tf=window->IFont)) {
         tf=(struct TextFont *)OpenFont(window->WScreen->Font);
         SetFont(window->RPort, tf);	/* set same font here */
         oflag=TRUE;
     }

   if(tf) {
        FontWidth=tf->tf_XSize;
        FontHeight=tf->tf_YSize;
   }
    else {
	FontWidth=8;
	FontHeight=8;
    }
    awidth=window->BorderLeft+window->BorderRight;
    aheight=window->BorderTop+window->BorderBottom;
    term.t_nrow=
      ((window->Height-aheight)/FontHeight)-1;
    if(nw.Type == WBENCHSCREEN) {
       term.t_nrow -=2;
       term.t_ncol = 
    	   (window->Width - awidth)/FontWidth;
    }
    else term.t_ncol=window->Width/FontWidth;
    RightMargin = term.t_ncol;

    WindowLimits(window,
	(FontWidth+awidth)*4,(FontHeight+aheight)*3,~0,~0);
}

void Cleanup()
{
struct Process *process;

if(OrigDir)CurrentDir(OrigDir); 
process = (struct Process *)FindTask(NULL); /* reset error window */
process->pr_WindowPtr=NULL;

if(window){
    ClearPointer(window);
    ClearMenuStrip(window);
    CloseWindowSafely(window);
}
if(screen) {
    if(version>= 36)PubScreenStatus(screen,PSNF_PRIVATE);
    CloseScreen(screen);
}
if(pastenode)KillPaste();

if(GfxBase !=0) {
    if(oflag && tf) CloseFont(tf);
    CloseLibrary(GfxBase);
}
if(IntuitionBase) {
    if(RememberKey)FreeRemember(&RememberKey, TRUE);
    OpenWorkBench(); /* just in case */
    CloseLibrary(IntuitionBase);
}
if(copybuffer)FreeMem(copybuffer,2048);
if(rdargs)FreeArgs(rdargs);
}

#endif

void errorExit(error)
int error;
{
#if AMIGA
if(IntuitionBase) {
    DisplayBeep(NULL);
}


Cleanup();
#endif
exit(error);
}

#if AMIGA
int CXBRK()
{
return(0);
}

void SetForWindow()
{
	nw.Type = WBENCHSCREEN;
	nw.IDCMPFlags = CLOSEWINDOW|NEWSIZE|MENUPICK|MOUSEBUTTONS|INACTIVEWINDOW;
	nw.Flags=WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|WINDOWCLOSE|
		SMART_REFRESH|ACTIVATE;
	nw.Title = ns.DefaultTitle;
	nw.TopEdge=0;
}
#endif
