/* slider.c started in 02/91...  C. Scheppner CBM
 * requires linkage with several iff modules
 * see Makefile
 *
 * 37_23: now 8 font slots, for/next allocs/deallocs, brush masks freed
 *        tries CGTriumvirate or Times if font not found
 * 37_24: = alone now means 1/2 line feed
 *        add shared UserPort, showilbm facility, shadow2 and shadpen2
 * 37_25: call text render from within parser (for text IN macros)
 * 37_36: startable from a Project slidefile
 * 39.1 : support LoadRGB32() (NewIFF setcolors())under V39
 * 39.2 : SetRast() before blitting clean background
 *        NOTE - current code forces depth to at leastb 4
 * 39.3 : add some transitions
 * 39.4 : fiddle with transitions, change default height to 216 (from 208)
 * 39.5 : add WIDTH and HEIGHT keywords for blank backgrounds
 * 39.6 : add pubscreentofront and pubscreentoback commands
 * 39.7 : add wbpause and command line skip=n
 * 39.9 : fix fadeout
 * 39.10: Workbench WIDTH and HEIGHT keywords were swapped - fixed
 * 40.1 : SAS 6 and new (39.10) IFF Modules
 * 40.2 : Link with newer (39.11) IFF modules
 */

#include "iffp/ilbmapp.h"

#include <exec/libraries.h>
#include <dos/dostags.h>
#include <graphics/text.h>
#include <libraries/diskfonttag.h>
#include <workbench/startup.h>
#include <hardware/blit.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/icon_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/icon_pragmas.h>

#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif

char *vers = "\0$VER: slider 40.1";
char *Copyright = 
  "slider v40.1\n(c) Copyright 1992-93 Commodore-Amiga, Inc.  All Rights Reserved";
char *usage =
"slider [slidefile | HELP] [bgname | NOBG | LACEBG] [SAVE basename] [skip=n]\n"
"If nobg or lacebg, optional WIDTH=n HEIGHT=n (default 656x216 or 432)\n"
"By default, looks for slidefile \"slides\" and bgname \"background\"\n"
"WB Project slidefile, BGNAME=,WIDTH=,HEIGHT=,SKIP=, Bool AUTO,NOBG,LACEBG\n";


BOOL UseMyWbCon = FALSE;
UBYTE  *MyWbCon = "";

#define DO_UPDATE	1
#define DO_NOT_UPDATE	0

ULONG getslidefile(UBYTE *filename);
BOOL ContainsBinary(BPTR file);
int getLine(UBYTE *buf);
void parseBuf(UBYTE *buf);
void doText(UBYTE *buf);
int showIt(void);
int showPrev(void);
int showNext(void);
void chkmsg(void);
void skipSlide(UBYTE *buf);
LONG getbrush(UBYTE *name, int bi);
void blitbrush(struct RastPort *rp, SHORT x,SHORT y,
			struct ILBMInfo *ilbm, UBYTE *mask);
void doBg(int);
void doBox(UWORD x,UWORD y,UWORD w,UWORD h);
void doPause(void);
void doFPause(void);
void setFont(int n);
BOOL loadFont(int n, char *name, int size, ULONG dpi, ULONG style);
void fadeout(void);
void fadein(struct ILBMInfo *ilbmpal, BOOL DoUpdate);
void update(void);
LONG syncsys(UBYTE *command, BPTR other);
LONG asyncsys(UBYTE *command);

int getval(char *s);
void InstallUserPort(struct Window *win, struct MsgPort *mp, ULONG idcmpflags);
void RemoveUserPort(struct Window *win);
void StripIntuiMessages(struct Window *win, struct MsgPort *mp);
void getWbOpts(struct WBArg *);
BOOL ttTrue(char **toolarray, UBYTE *keyword);
void ScreenToFrontReset(struct Screen *scr);

void trans_none(void);
void wipe_lr(void);
void wipe_rl(void);
void wipe_tb(void);
void wipe_bt(void);
void slide_up(void);

void cleanup(void);
void bye(UBYTE *s,int error);

/* our function error codes */
#define SYSTEMFAIL	(-1L)
#define WINDOWFAIL	(-2L)
#define VERSIONFAIL	(-3L)

ULONG stags[] = { SA_PubName, (ULONG)"SLIDER", TAG_DONE };

char *helplines[] = {
"Slidefile is an ASCII text file which includes slider commands.",
"Slider will only interpret lines starting with the following characters:",
".SS (slide start)  .SE (slide end)  .SC (means slide command)",
"Plain text lines between a .SS and a .SE will be rendered.",
"Slider Commands (l=lines  #=number): leftmargin # (def 12), topmargin #",
"vertspace # (def 8), charspace #, center l, pen #, bgpen #",
"shadow[2] w h xoff yoff, shadpen[2] #, savepen, restorepen",
"bullet l, bulletpen #, bulletstring string (no spaces), bulletoffs x y",
"bulletbrush filename (to use brush as bullet instead of bulletstring)",
"loadfont 1-8 name.font fontsize [XDPI YDPI] [italic bold soft], fontnum 1-8",
"vertmove #, box x y w h (y0=next,w0=centered), setpointer on|off, pause",
"update (force update of visible display), syncsys command, asyncsys command",
"macro macroname, endm (intervening .SC commands define the macro).",
"loadbg <1|2> filename, setbg <0|1|2>, dobg <0|1|2>, notes, endnotes",
"addbrush filename x y [jam2], palette 0xhhh [0xhhh...], fade on|off",
"settrans none/wipe_lr/wipe_rl/wipe_tb/wipe_bt/slide_up, showilbm filename",
"pubscreentofront/pubscreentoback [screenname], wbpause",
" > Default ILBM background name for bg 1 is \"background\".",
" > Save option saves slides as basenameN ILBMs starting with N=0001",
" > Initial char @ means a bullet, @@ invis bullet",
" > Only char - is 1/4 LF, = is 1/2 LF",
"" };

#define LMARGIN 12
#define TMARGIN 0
#define VERTSPACE 8
#define CHARSPACE 0

#define  XWIDE 656
#define  YHIGH 216
#define  DMODE (HIRES)
#define  BDEEP 4

ULONG   swidth, sheight;

#define LBUFSZ    160
#define TBUFSZ    40

BOOL     Done;
ULONG    code,class,mousex,mousey,delay,pic;
int      error;

struct Library *IntuitionBase  = NULL;
struct Library *GfxBase        = NULL;
struct Library *DiskfontBase   = NULL;
struct Library *IFFParseBase   = NULL;
struct Library *IconBase       = NULL;

WORD	 topedge, leftedge;	/* for resetting top/left edge if dragged */

struct   Screen		*pubscreen;
struct   Screen         *hscr, *vscr;         /* for ptr to screen structure */
struct   Window         *hwin, *vwin, *iwin;  /* for ptr to window structure */

struct   RastPort       *hrp, *vrp;           /* for ptr to RastPort  */
struct   ViewPort       *hvp, *vvp;           /* for ptr to Viewport  */

struct   IntuiMessage   *msg;


struct   TextAttr       MyFont = {
   "topaz.font",                    /* Font Name   */
   TOPAZ_EIGHTY,                    /* Font Height */
   FS_NORMAL,                       /* Style       */
   FPF_ROMFONT                      /* Preferences */
   };


struct   NewScreen      ns = {
   0, 0,                                  /* LeftEdge and TopEdge   */
   XWIDE,YHIGH,                           /* Width and Height       */
   BDEEP,                                 /* Depth                  */
   0, 1,                                  /* DetailPen and BlockPen */
   DMODE,                                 /* Special display modes  */
   CUSTOMSCREEN,                          /* Screen Type            */
   &MyFont,                               /* Use my font            */
   " Slider - by C. Scheppner ",          /* Title                  */
   NULL,                                  /* No gadgets yet         */
   NULL,                                  /* No bitmap supplied     */
   };

#define IFLAGS	(MOUSEBUTTONS|VANILLAKEY)

struct   NewWindow      mynw = {
   0, 0,                                  /* LeftEdge and TopEdge */
   0, 0,                          	  /* Width and Height */
   (UBYTE)-1, (UBYTE)-1,                  /* DetailPen and BlockPen */
   NULL,                                  /* IDCMP Flags with Flags below */
   BACKDROP|BORDERLESS|SMART_REFRESH|NOCAREREFRESH|ACTIVATE|RMBTRAP,
   NULL, NULL,                            /* Gadget and Image pointers */
   NULL,                                  /* Title string */
   NULL,                                  /* Screen ptr null till opened */
   NULL,                                  /* BitMap pointer */
   50, 20,                                /* MinWidth and MinHeight */
   0 , 0,                                 /* MaxWidth and MaxHeight */
   CUSTOMSCREEN                           /* Type of window */
   };

#define NUMFONTS 8

ULONG fonttags[] = { TA_DeviceDPI, ((20L << 16)|(20)), TAG_DONE };
struct TTextAttr ttAttrs[NUMFONTS] = {0};
struct TextFont *fonts[NUMFONTS] = {NULL};
ULONG  fontstyles[NUMFONTS] = {0L};

LONG   rLen, syserror;

UBYTE *transnames[] = {
 "none",
 "wipe_lr","wipe_rl","wipe_tb","wipe_bt",
 "fade_black","fade_bg","slide_up",
 NULL };

#define TRANS_NONE	0
#define TRANS_WIPE_LR	1
#define TRANS_WIPE_RL	2
#define TRANS_WIPE_TB	3
#define TRANS_WIPE_BT	4
#define TRANS_FADE_BLACK 5
#define TRANS_FADE_BG    6
#define TRANS_SLIDE_UP   7

int trans = TRANS_NONE;

/* default initial font */
UBYTE  *defbitfontname = "Times.font";
UBYTE  *defbulfontname = "CGTriumvirate.font";
UBYTE  *fontname = "Times.font";
UWORD  fontsize = 24;
ULONG  fontdpi = ((20L<<16)|20);
UWORD  baseline;
UBYTE fontnamebuf[48];
UBYTE bgnamebuf[128];

UWORD  defcolors[32] = 
	{ 0x668,0xFFF,0x000,0xF50,0xFD0,0xFF0,0x779,0x557,
  	  0xEEC,0xD5F,0x0E5,0xCCF,0xFE7,0xCCC,0x888,0x444 };

UBYTE  fontflags = 0;

UBYTE  lbuf[LBUFSZ];
BOOL   FromWb;



/* Property chunks to be grabbed */
LONG	ilbmprops[] = {
		ID_ILBM, ID_BMHD,
		ID_ILBM, ID_CMAP,
		ID_ILBM, ID_CAMG,
		ID_ILBM, ID_CCRT,
		ID_ILBM, ID_AUTH,
		ID_ILBM, ID_Copyright,
		TAG_DONE
		};

/* Collection chunks to be gathered */
LONG	ilbmcollects[] = {
		ID_ILBM, ID_CRNG,
		TAG_DONE
		};

/* Chunk to stop on */
LONG	ilbmstops[] = {
		ID_ILBM, ID_BODY,
		TAG_DONE
		};

UBYTE nomem[]  = "Not enough memory\n";
UBYTE noiffh[] = "Can't alloc iff\n";

#define DEF	0
#define HID	1
#define VIS	2
#define SHO	3
#define BRU	4
#define BUL	5
#define ILBMCNT 6
struct ILBMInfo *ilbm[ILBMCNT] = { NULL };

UBYTE *brumask[ILBMCNT] = {0};
ULONG brumaskbytes[ILBMCNT] = {0};

UBYTE bruname[80] = "";
UBYTE bulname[80] = "";
/* first 3 are dummies for safety */
UBYTE *brunames[ILBMCNT] =
   { bruname, bruname, bruname, bruname, bruname, bulname };

#define MAXSLIDES 100
LONG slides[MAXSLIDES] = {0};
UWORD slidecnt = 0;
ULONG slidenum, maxslidenum;

#define ENDMACROS (0xFFFF)

UBYTE *SLIDE_START = ".SS";
UBYTE *SLIDE_END   = ".SE";
UBYTE *SLIDE_COM   = ".SC";
UBYTE *SLIDE_MACRO = ".SC macro";
UBYTE *SLIDE_ENDM  = ".SC endm";

UBYTE *filename = "slides";
UBYTE *bgname = "background";
UBYTE *dest = "DEST:slide";
UBYTE *names[3];

UWORD shadw = 2, shadh = 2, shadx = 2, shady = 2;
UWORD shadw2 = 0, shadh2 = 0, shadx2 = 0, shady2 = 0;

BOOL  BgDone = FALSE, GoinBack = FALSE, FadeOn = FALSE, FadedOut=FALSE;
BOOL  SlidUp = FALSE;
BOOL  Save = FALSE, Auto = FALSE, NotesOn = FALSE, NoBg = FALSE;
BOOL  BulletBrush = FALSE;
UWORD pen = 1, bgpen = 0, shadpen = 2, shadpen2 = 0, bulletpen = 3, pushpen = 1;
ULONG center = 0;
UWORD bgnum = DEF;
UWORD boxx, boxy, boxw, boxh;
UWORD bullet, bulletxoffs, bulletyoffs;
#define BULLCHARS 32
UBYTE bulletstring[BULLCHARS] = {"-"};

SHORT lmargin = LMARGIN, tmargin = TMARGIN;
SHORT vertspace = VERTSPACE, charspace = CHARSPACE; 
SHORT tx, ty;

/* for the loaded in slide file */
UBYTE *linetext=NULL, **linearray=NULL;
ULONG *smacs = NULL;
ULONG ltsz = NULL, lasz = NULL, smsz=NULL, lindex=NULL, linecnt, mcnt;
LONG tickdelay = 400, skipto=0;

/* For alloc to define new pointer */
#define PDATASZ 12
UWORD   *pdata;
		
BPTR startLock = NULL;

/* 
 * MAIN 
 */
void main(int argc, char **argv)
   {
   struct IntuiMessage *stopmsg;
   BitMapHeader	*bmhd;
   ULONG mode, u;
   LONG error;
   int k,n,saven;
   UBYTE  savename[128];
   struct WBStartup *WBenchMsg;
   struct WBArg     *wbargs, *arg;

   FromWb = argc ? FALSE : TRUE;

   if(FromWb)
	{
	WBenchMsg = (struct WBStartup *)argv;
	if(WBenchMsg->sm_NumArgs == 1)
	    {
		printf("%s\n%s\n",Copyright,usage);
		for(k=0; *helplines[k]; k++) printf("%s\n",helplines[k]);
		bye(" ",RETURN_OK);
	    }
	}

   if(argc == 2)
	{
	if(argv[1][0]=='?')
		{
		printf("%s\n%s\n",Copyright,usage);
        	bye("",RETURN_OK);
		}
	else if(!stricmp(argv[1],"help"))
		{
		printf("%s\n%s\n",Copyright,usage);
		for(k=0; *helplines[k]; k++) printf("%s\n",helplines[k]);
		bye("",RETURN_OK);
		}
	}


   /* Options */
   saven = 1;

   names[0] = filename;
   names[1] = bgname;
   names[2] = dest;
   swidth = XWIDE;
   sheight = YHIGH;

   if(FromWb)
	{
        wbargs = WBenchMsg->sm_ArgList;
        /* The Workbench arg of the Display Tool */
        arg = &wbargs[0];
        startLock  = (BPTR)CurrentDir(arg->wa_Lock);
        /* get program's ToolTypes */
        getWbOpts(arg);

        /* Now the project (we know we have at least one here) */
        arg = &wbargs[1];
	names[0] = arg->wa_Name;
        /* CD to where this Project file is */
        CurrentDir(arg->wa_Lock);
        /* Get Project's ToolTypes and name */
	getWbOpts(arg);
	}
   else for(k=1,n=0; k<argc; k++)
	{
	if(!(stricmp(argv[k],"SAVE")))
	    {
	    Auto = TRUE;
	    Save = TRUE;
	    if(argc > (k+1)) names[2] = argv[++k];
	    }
	else if(!(stricmp(argv[k],"AUTO")))
	    {
	    Auto = TRUE;
	    }
	else if(!(stricmp(argv[k],"NOBG")))
	    {
	    NoBg = TRUE;
	    }
	else if(!(stricmp(argv[k],"LACEBG")))
	    {
	    NoBg = TRUE;
	    ns.ViewModes |= LACE;
	    if(ns.Height == YHIGH)	ns.Height = YHIGH << 1;
	    sheight = ns.Height;
	    }
	else if(!(strnicmp(argv[k],"BGNAME=",7)))
	    {
	    names[1] = &argv[k][7];
	    }
	else if(!(strnicmp(argv[k],"HEIGHT=",7)))
	    {
	    if(u = atoi(&argv[k][7]))
		{
		sheight = ns.Height = u;
		if(sheight >= 400)	ns.ViewModes |= LACE;
		}
	    else printf("invalid screen height: %ld\n",u);
	    }
	else if(!(strnicmp(argv[k],"WIDTH=",6)))
	    {
	    if(u = atoi(&argv[k][6]))
		{
		swidth = ns.Width = u;
	    	if(swidth >= 640)	ns.ViewModes |= HIRES;
	    	else			ns.ViewModes &= (~HIRES);
		}
	    else printf("invalid screen width: %ld\n",u);
	    }
	else if(!(strnicmp(argv[k],"SKIP=",5)))
	    {
	    if(u = atoi(&argv[k][5])) skipto = u;
	    }
        else
	    {
	    names[n] = argv[k];
	    n++;
	    }
	}
   filename = names[0];
   bgname   = names[1];
   dest     = names[2];

/*
   printf("Names are %s %s %s, Save=%ld Auto=%ld\n",
		filename, bgname, dest, Save, Auto );
   exit(RETURN_OK);
*/

   /* Open Libraries */

   if(!(IntuitionBase = OpenLibrary("intuition.library", 0)))
      bye("Can't open intuition library.\n",RETURN_WARN);
      
   if(!(GfxBase = OpenLibrary("graphics.library",0)))
      bye("Can't open graphics library.\n",RETURN_WARN);

   if(!(DiskfontBase = OpenLibrary("diskfont.library",0)))
      bye("Can't open diskfont library.\n",RETURN_WARN);

   if(!(IFFParseBase = OpenLibrary("iffparse.library",0)))
      bye("Can't open iffparse library.\n",RETURN_WARN);

   if(!(pdata = (UWORD *)AllocMem(PDATASZ,MEMF_CHIP|MEMF_CLEAR)))
	bye("Can't alloc clear pointer",RETURN_FAIL);

   if(!(linecnt=getslidefile(filename)))
	{
	printf("Can't open slides file \"%s\"\n",filename);
      	bye(" ",RETURN_WARN);
	}

    
/* Allocate ilbm structures
 */
    if(!(ilbm[0] = (struct ILBMInfo *)
	AllocMem(ILBMCNT * sizeof(struct ILBMInfo),MEMF_PUBLIC|MEMF_CLEAR))) 
		bye(nomem,RETURN_FAIL);
    else 
	{
	for(k=1; k<ILBMCNT; k++) ilbm[k] = ilbm[0] + k;
	}
/*
 * Set up default ILBMInfo and init the others from it
 */
    ilbm[DEF]->ParseInfo.propchks	= ilbmprops;
    ilbm[DEF]->ParseInfo.collectchks	= ilbmcollects;
    ilbm[DEF]->ParseInfo.stopchks	= ilbmstops;
    ilbm[DEF]->windef			= &mynw;

    for(k=1; k<ILBMCNT; k++) *ilbm[k] = *ilbm[DEF];

    ilbm[DEF]->colortable = defcolors;
    ilbm[VIS]->stags = stags;

/* 
 * Now alloc the IFF handles (so we can load one while displaying another) 
 */
    for(k=1; k<ILBMCNT; k++)
	{
    	if(!(ilbm[k]->ParseInfo.iff = AllocIFF())) bye(noiffh,RETURN_FAIL);
	}

    mode = ns.ViewModes;

    if((NoBg)||(error = loadbrush(ilbm[HID],bgname)))
	{
	bgnum = DEF;
	if(!NoBg) printf("Can't load background \"%s\"\n",bgname);
	bmhd = NULL;
	}
    else
	{
	bgnum = HID;
    	bmhd  = &ilbm[HID]->Bmhd;
	mode  = ilbm[HID]->camg; 
	}	

    if(bgnum != DEF)
	{
    	D(bug("bmhd0 w=%ld h=%ld\n",ilbm[HID]->Bmhd.w,ilbm[HID]->Bmhd.h));

   	/* Open Screen, Window */
   	ns.Depth  = MAX(bmhd->nPlanes,BDEEP);
   	ns.Width  = bmhd->w;
   	ns.Height = bmhd->h;
        ns.ViewModes   = ilbm[HID]->camg;
	}

   /* hidden screen for rendering */
   ilbm[HID]->stype = CUSTOMSCREEN|SCREENBEHIND;
   if(!(hwin=opendisplay(ilbm[HID],ns.Width,ns.Height,ns.Depth,mode)))
      bye("Can't open screen.\n",RETURN_WARN);
   ilbm[HID]->win = hwin;
   ilbm[HID]->scr = hscr = hwin->WScreen;
   ilbm[HID]->vp  = hvp  = &hwin->WScreen->ViewPort;
   hrp = hwin->RPort;

   /* visible screen */
   if(!(vwin=opendisplay(ilbm[VIS],ns.Width,ns.Height,ns.Depth,mode)))
      bye("Can't open screen.\n",RETURN_WARN);
   ilbm[VIS]->win = vwin;
   ilbm[VIS]->scr = vscr = vwin->WScreen;
   ilbm[VIS]->vp  = vvp  = &vwin->WScreen->ViewPort;
   vrp = vwin->RPort;

   SetPointer(vwin,pdata,1,16,0,0);

   /* save for repositioning if necessary later */
   leftedge = vscr->LeftEdge;
   topedge  = vscr->TopEdge;

   /* Add UserPort to visible window, shared with other window */
   ModifyIDCMP(vwin,IFLAGS);
   if(!vwin->UserPort)	bye("Can't create window UserPort",RETURN_FAIL);
   InstallUserPort(hwin, vwin->UserPort, IFLAGS);


   /* Last 16 Default colors */
   ilbm[DEF]->ncolors = 32;
   for(k=17; k<32; k++) ilbm[DEF]->colortable[k] = GetRGB4(vvp->ColorMap,k);

   /* Colors in use */
   setcolors(ilbm[bgnum],vvp);
   setcolors(ilbm[bgnum],hvp);

   for(k=0; k<NUMFONTS; k++) loadFont(k,fontname,fontsize,fontdpi,0L);

   if(fonts[0]) setFont(0);
   
   showNext();
   
   Done = FALSE;
   while(!Done)
      {
      update();
      if(!Auto)
	{
      	Wait(1<<vwin->UserPort->mp_SigBit);
      	chkmsg();
	}
      else /* Is Auto */
	{
	if(Save)
	    {
	    sprintf(savename,"%s%04ld",dest,saven++);
	    if(error = screensave(ilbm[VIS], vscr, NULL, NULL, savename))
		printf("Error saving \"%s\"\n",savename);
	    }
	else Delay(tickdelay);
                     
	if(stopmsg = (struct IntuiMessage *)GetMsg(vwin->UserPort))
	    {
	    if((stopmsg->Class == VANILLAKEY)&&((stopmsg->Code=='q')||
			(stopmsg->Code==0x03)||(stopmsg->Code==0x04)))
		{
		Save=FALSE;
		Auto=FALSE;
		}
	    }
	/* If still auto, showNext and check if done */ 
	if(Auto) Done = showNext() ? FALSE : TRUE;
	/* If just auto (not save) and done, loop back to beginning */
	if((Done)&&(Auto)&&(!Save))
	    {
	    Done = FALSE;
	    lindex = 0;
	    slidenum = 0;
	    showNext();
	    }
	 }
      }

   cleanup();
   exit(RETURN_OK);
   }


BOOL ContainsBinary(BPTR file)
   {
   int k, rLen;
   UBYTE buf[80], ch;

   if((rLen = Read(file, buf, 80))>0)
	{
   	for (k=0; k<rLen; k++)
      	    {
      	    ch = buf[k];
      	    if(((ch>0x00)&&(ch<0x08))||
       		  ((ch>0x0d)&&(ch<0x1b))||
       		  ((ch>0x1b)&&(ch<0x20))||
       		  ((ch>0x7f)&&(ch<0xa0)))
		{
		printf("Slide file appears to contain binary\n");
		if((buf[0]=='F')&&(buf[1]=='O')&&(buf[2]=='M')&&(buf[3]=='M'))
			printf("You may have placed ILBM arg before slides arg\n");
		return(TRUE);
		}
      	    }
	}
   return(FALSE);
   }


void chkmsg(void)
    {
    struct Window *mwin;

    while(msg = (struct IntuiMessage *)GetMsg(vwin->UserPort))
    	{
      	class = msg->Class;
      	code = msg->Code;
      	mousex = msg->MouseX;
      	mousey = msg->MouseY;
      	mwin = msg->IDCMPWindow;
      	ReplyMsg(msg);

      	/* If clicked/typed on hidden window, bring visible to front */
      	if(mwin == hwin)
	    {
	    ScreenToFrontReset(vscr);
	    ActivateWindow(vwin);
	    }

      	switch(class)
            {
            case CLOSEWINDOW:
            	Done = TRUE;
            	break;
	    case MOUSEBUTTONS:
	    	switch(code)
		    {
		    case SELECTDOWN:
		        if((mousex > 4)&&(mousex < 10)&&
		            (mousey > 4)&&(mousey < 10))	Done = TRUE;
		   	else showNext();
		   	break;
		    case MENUDOWN:
		   	showPrev();
		   	break;
		    default:
		   	break;
		    }
            case VANILLAKEY:
            	switch(code)
               	    {
                    case 'n': case ' ': case 0x0a: case 0x0d:
                  	showNext();
                  	break;
                    case 'b': case 0x08:
                  	showPrev();
                  	break;
	            case '<':
		  	lindex = 0;
		  	slidenum = 0;
		  	showNext();
		  	break;
	            case '>':
		  	slidenum = maxslidenum + 1;
		  	showPrev();
		  	break;
	            case 'f':  /* fadeout and pause */
		 	doFPause();
		  	break;
               	    case 'q': case 0x04: case 0x03:
                  	Done = TRUE;
                  	break;
                    default:
                  	break;
               	    }
         	default:
            	    break;
            }
	}
    }



int showNext(void)
   {
   int success;

   GoinBack = FALSE;

   if(FadeOn && (!FadedOut))	fadeout();

   success = showIt();
   if(success)
	{
	slidenum++;
	if(maxslidenum < slidenum)  maxslidenum=slidenum;
	}
   return(success);
   }

int showPrev(void)
   {
   int k;

   if(slidenum > 1)
	{
   	GoinBack = TRUE;
	slidenum--;
	}

   lindex = 0;
   for(k=1; k<slidenum; k++)
	{
	skipSlide(lbuf);
	}
   return(showIt());
   }


void parseBuf(UBYTE *buf)
    {
    UBYTE *p = buf;
    UBYTE token[TBUFSZ];
    LONG error;
    ULONG li, jammode, dpi;
    int k, l, lt, ll, mm, n, size, x, y, soft, bi;

    if(*buf != '.')
	{
	doText(buf);
	return;
	}

    p = stptok(p,token,TBUFSZ," ,");
    if(strnicmp(token,SLIDE_COM,strlen(SLIDE_COM)))	return;

    /* Have a SLIDE_COM command - get command name */
    p = stpblk(p);
    p = stptok(p,token,TBUFSZ," ,");

    /* skip over macro definition lines when parsing */
    if(!(stricmp("macro" ,token)))
	{
	l = strlen(SLIDE_ENDM);
	while(strnicmp(buf,SLIDE_ENDM,l))   getLine(buf);
	}
    /* built-in commands */
    else if(!(stricmp("pen",token)))	     pen    = atoi(p);
    else if(!(stricmp("bgpen",token)))	     bgpen  = atoi(p);
    else if(!(stricmp("savepen",token)))     pushpen = pen;
    else if(!(stricmp("restorepen",token)))  pen = pushpen;
    else if(!(stricmp("shadpen",token)))     shadpen   = atoi(p);
    else if(!(stricmp("shadpen2",token)))    shadpen2  = atoi(p);
    else if(!(stricmp("center",token)))      center = atoi(p);
    else if(!(stricmp("bullet",token)))      bullet = atoi(p);
    else if(!(stricmp("bulletpen",token)))   bulletpen = atoi(p);
    else if(!(stricmp("bulletoffs",token)))  
	{
	bulletxoffs = atoi(p);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	bulletyoffs = atoi(p);
	}
    else if(!(stricmp("bulletstring",token)))
	{
	BulletBrush = FALSE;
	bulletstring[BULLCHARS-1] = '\0';
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	strncpy(bulletstring,token,BULLCHARS-1);
/*
	printf("strlen bulletstring = %ld, token is \"%s\", string is \"%s\"\n",
			strlen(bulletstring),token,bulletstring);
*/
	}
    else if(!(stricmp("leftmargin",token)))  lmargin = atoi(p);
    else if(!(stricmp("topmargin",token)))   tmargin = atoi(p);
    else if(!(stricmp("vertspace",token)))   vertspace = atoi(p);
    else if(!(stricmp("fadeout",token)))     fadeout();
    else if(!(stricmp("fadein",token)))	     fadein(ilbm[bgnum],DO_UPDATE);
    else if(!(stricmp("update",token)))      update();
    else if(!(stricmp("charspace",token)))   charspace = atoi(p);
    else if(!(stricmp("vertmove",token)))    ty += atoi(p);
    else if(!(stricmp("notes",token)))	     NotesOn = TRUE;
    else if(!(stricmp("endnotes",token)))    NotesOn = FALSE;
    else if(!(stricmp("pubscreentofront",token)))
	{
	if(IntuitionBase->lib_Version >= 37)
	    {
	    if(*p) p = stpblk(++p);
	    if(pubscreen = LockPubScreen((*p) ? p : NULL))
		{
		ScreenToFront(pubscreen);
		UnlockPubScreen(NULL,pubscreen);
		}
	    }
    	}
    else if(!(stricmp("pubscreentoback",token)))
	{
	if(IntuitionBase->lib_Version >= 37)
	    {
	    if(*p) p = stpblk(++p);
	    if(pubscreen = LockPubScreen((*p) ? p : NULL))
		{
		ScreenToBack(pubscreen);
		UnlockPubScreen(NULL,pubscreen);
		}
	    }
	}
    else if(!(stricmp("setpointer",token)))
	{
	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	if(!(stricmp(token,"on")))	ClearPointer(vwin);
  	else				SetPointer(vwin,pdata,1,16,0,0);
	}
    else if(!(stricmp("syncsys",token)))
	{
    	p = stpblk(++p);
	syserror = syncsys(p,NULL);
	}
    else if(!(stricmp("asyncsys",token)))
	{
    	p = stpblk(++p);
	syserror = asyncsys(p);
	}
    else if(!(stricmp("fade",token)))
	{
    	p = stpblk(++p);
	if(!(stricmp(p,"on")))	FadeOn = TRUE;
	else
		{
		FadeOn = FALSE;
		if(FadedOut) fadein(ilbm[bgnum],DO_UPDATE);
		}
	}
    else if((!(stricmp("dobg",token)))||(!(stricmp("setbg",token))))
	{
	n = bgnum;
    	p = stpblk(++p);
	if((*p)&&(*p != ' ')) n = atoi(p);
	if((n<0)||(n>2))	n=0;
	if((n==DEF)||(ilbm[n]->brbitmap)) bgnum = n;
	else printf("Warning - bg %ld not available\n",n);
	if(!(stricmp("dobg",token))) doBg(bgnum);
	}
    else if(!(stricmp("loadfont",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	n = atoi(token);
	if(n >= NUMFONTS)	n=0;
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	strcpy(fontnamebuf,token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	size = atoi(token);
	fontstyles[n] = 0;
	dpi = 0L;
	soft = 0;
	while(*p)
	    {
    	    p = stptok(stpblk(++p),token,TBUFSZ," ,");
	    if(!(stricmp(token,"italic")))  fontstyles[n] |= FSF_ITALIC;
	    else if(!(stricmp(token,"bold")))    fontstyles[n] |= FSF_BOLD;
	    else if(!(stricmp(token,"soft")))    soft = 1;
	    else if(atoi(token)) dpi = (dpi << 16) | (atoi(token));
	    }
	loadFont(n, fontnamebuf, size, dpi, soft ? 0L : fontstyles[n]);
	}
    else if(!(stricmp("fontnum",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	n = atoi(token);
	if(n < NUMFONTS)
		{
		setFont(n);
		}
	else printf("warning - fontnum too high (>3)\n");
	}
    else if(!(stricmp("settrans",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	for(k=0,trans=0; transnames[k]; k++)
	    {
	    if(!(stricmp(token,transnames[k])))
		{
		trans = k;
		break;
		}
	    }
	}
    else if((!(stricmp("pause",token)))&&(!GoinBack))
	{
	update();
	doPause();
	}
    else if((!(stricmp("wbpause",token)))&&(!GoinBack))
	{
	update();
	if(IntuitionBase->lib_Version >= 37)
	    {
	    if(pubscreen = LockPubScreen("Workbench"))
		{
		ScreenToFront(pubscreen);
		UnlockPubScreen(NULL,pubscreen);
		}
	    }
	doPause();
	}
    else if(!(stricmp("box",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	boxx = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	boxy = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	boxw = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	boxh = atoi(token);

	if(!boxy) boxy = ty + 8;
	if(!boxw) boxw = (hwin->Width) - (boxx << 1);
	if(!boxh) boxh = (hwin->Height)- (boxy << 1);
	doBox(boxx, boxy, boxw, boxh);
	}
    else if(!(stricmp("shadow",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shadw = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shadh = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shadx = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shady = atoi(token);
	}
    else if(!(stricmp("shadow2",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shadw2 = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shadh2 = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shadx2 = atoi(token);
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	shady2 = atoi(token);
	}
    else if(!(stricmp("palette",token)))
	{
	n = 0;
	while((*p)&&(n<32))
	    {
    	    p = stptok(stpblk(++p),token,TBUFSZ," ,");
   	    defcolors[n] = getval(token);
	    n++;
	    }
   	setcolors(ilbm[bgnum],hvp);
   	setcolors(ilbm[bgnum],vvp);
	}
    else if(!(stricmp("addbrush",token)))
	{
	bi = BRU;
    	p = stptok(stpblk(++p),token,TBUFSZ," ,"); /* get name */

	if(!(error=getbrush(token,bi)))
	    {
    	    p = stptok(stpblk(++p),token,TBUFSZ," ,");
	    x = atoi(token);
    	    p = stptok(stpblk(++p),token,TBUFSZ," ,");
	    y = atoi(token);
	    jammode = JAM1;
	    if(*p)
		{
    		p = stptok(stpblk(++p),token,TBUFSZ," ,");
		if(!(stricmp(token,"JAM2")))	jammode = JAM2;
		}
	    blitbrush(hrp, (SHORT)x, (SHORT)y, ilbm[bi],(jammode==JAM1 ? brumask[bi] : 0));
	    }
	}
    else if(!(stricmp("bulletbrush",token)))
	{
	bi = BUL;
    	p = stptok(stpblk(++p),token,TBUFSZ," ,"); /* get name */

	if(!(error=getbrush(token,bi)))
	    {
	    BulletBrush = TRUE;
	    }
	}
    else if(!(stricmp("loadbg",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	n = atoi(token);
	if((n<1)||(n>2))	printf("n can't be %ld\n",n), n=1;
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	if(ilbm[n]->brbitmap)	unloadbrush(ilbm[n]);
    	if(error = loadbrush(ilbm[n],token))
		{
		printf("Warning - background %s load failed\n",token);
		if(bgnum == n)	bgnum = DEF;
		}
	}
    else if(!(stricmp("showilbm",token)))
	{
    	p = stptok(stpblk(++p),token,TBUFSZ," ,");
	mynw.Flags &= (~ACTIVATE);
    	if(error = showilbm(ilbm[SHO],token))
		{
		printf("Warning - display of ILBM %s failed\n",token);
		}
	else
		{
		iwin = ilbm[SHO]->win;
		SetPointer(iwin,pdata,1,16,0,0);
		ActivateWindow(iwin);
		ModifyIDCMP(iwin, IFLAGS);
		ScreenToFrontReset(iwin->WScreen);
		if(iwin->UserPort)	WaitPort(iwin->UserPort);
		else
			{
			printf("Can't create Port for ILBM window - delaying\n");
			Delay(500);
			}
		unshowilbm(ilbm[SHO]);
		ilbm[SHO]->scr = NULL;
		ilbm[SHO]->win = NULL;
		iwin = NULL;
		}
	}

    /* command may be a macro */
    else
	{
	lt = strlen(token);
	for(mm=0; mm<mcnt; mm++)
	    {
	    li = smacs[mm];
	    ll = strlen(linearray[li]);	    
	    if((ll>lt)&&(!(strcmp(token,&linearray[li][ll-lt]))))
		{
		/* found macro - move to first command line of macro */
		li++;
		l = strlen(SLIDE_ENDM);
		/* Until we reach endm, parse/do the lines of the macro
		 * Note reentrancy
		 */
		while(strnicmp(linearray[li],SLIDE_ENDM,l))
		    {
		    strcpy(buf,linearray[li]);
		    parseBuf(buf);
		    li++;
		    }
		/* update(); */
		mm=mcnt;	/* terminate the search */
		}
	    }	    
	}
    }


void doPause()
    	{
	struct IntuiMessage *msg;
	SHORT code;
	BOOL Pausing;

	if(Auto)	return;

	Pausing = TRUE;
	while(Pausing)
	    {
	    WaitPort(vwin->UserPort);
	    msg = (struct IntuiMessage *)GetMsg(vwin->UserPort);
	    code = msg->Code;
	    if((msg->Class == VANILLAKEY)&&
	       ((code=='q')||(code==0x03)||(code==0x04)))
		bye("",RETURN_OK);

	    else if(msg->Class == VANILLAKEY)
		{
		if(code =='f')	doFPause();
		else Pausing=FALSE;
		}
	    else if((msg->Class == MOUSEBUTTONS)&&(code==SELECTDOWN))
		 Pausing=FALSE;
	    ReplyMsg(msg);
	    }
	}

void doFPause()
    	{
	struct IntuiMessage *msg;

	if(Auto)	return;

	fadeout();
	WaitPort(vwin->UserPort);
	msg = (struct IntuiMessage *)GetMsg(vwin->UserPort);
	ReplyMsg(msg);
	fadein(ilbm[bgnum],DO_NOT_UPDATE);
	}

void update()
    {
    switch(trans)
	{
	case TRANS_NONE:
	    trans_none();
	    break;
	case TRANS_WIPE_LR:
	    wipe_lr();
	    break;
	case TRANS_WIPE_RL:
	    wipe_rl();
	    break;
	case TRANS_WIPE_TB:
	    wipe_tb();
	    break;
	case TRANS_WIPE_BT:
	    wipe_bt();
	    break;
	case TRANS_FADE_BLACK:
	    trans_none();
	    break;
	case TRANS_FADE_BG:
	    trans_none();
	    break;
	case TRANS_SLIDE_UP:
	    if(!SlidUp)
		{
	    	slide_up();
		SlidUp = TRUE;
		}
	    else trans_none();
	    break;
	default:
	    trans_none();
	    break;
	}
    }

void trans_none()
    {
    /* Bring hidden to front */
    ScreenToFrontReset(hscr);
    /* Blit HID to VIS */
    BltBitMapRastPort(hrp->BitMap,0,0,
		     vrp,0,0,
		     hscr->Width, hscr->Height,
		     0xC0);

    /* Copy HID colors to VIS */
    /* WARNING - used to get colors right from colortable of hidden viewport */
    if(!FadedOut)	setcolors(ilbm[HID],vvp);
    /* bring VIS to front */
    ScreenToFrontReset(vscr);

    if(FadedOut)  fadein(ilbm[bgnum],DO_NOT_UPDATE); /* update would recurse */
    }

void wipe_lr()
    {
    int x,step;

    step = hscr->Width / 32;
    ScreenToFrontReset(vscr);   /* Make sure visible screen is in front */

    for(x=0; x<hscr->Width; x+=step)
	{
	WaitBOVP(vvp);
    	/* Blit HID to VIS */
    	BltBitMapRastPort(hrp->BitMap,x,0, vrp,x,0, step, hscr->Height, 0xC0);
	}
    }

void wipe_rl()
    {
    int x,step;

    step = hscr->Width / 32;
    ScreenToFrontReset(vscr);   /* Make sure visible screen is in front */

    for(x=hscr->Width-step; x>(-step); x-=step)
	{
	WaitBOVP(vvp);
    	/* Blit HID to VIS */
    	BltBitMapRastPort(hrp->BitMap,x,0, vrp,x,0, step, hscr->Height, 0xC0);
	}
    }

void wipe_tb()
    {
    int y,step;

    step = hscr->Height / 24;
    ScreenToFrontReset(vscr);   /* Make sure visible screen is in front */

    for(y=0; y<hscr->Height; y+=step)
	{
	WaitBOVP(vvp);
    	/* Blit HID to VIS */
    	BltBitMapRastPort(hrp->BitMap,0,y, vrp,0,y, hscr->Width, step, 0xC0);
	}
    }

void wipe_bt()
    {
    int y,step;

    step = hscr->Height / 24;
    ScreenToFrontReset(vscr);   /* Make sure visible screen is in front */

    for(y=hscr->Height-step; y>(-step); y-=step)
	{
	WaitBOVP(vvp);
    	/* Blit HID to VIS */
    	BltBitMapRastPort(hrp->BitMap,0,y, vrp,0,y, hscr->Width, step, 0xC0);
	}
    }

void slide_up()
    {
    int step;

    step = hscr->Height / 10;
    ScreenToFrontReset(vscr);   /* Make sure visible screen is in front */

    MoveScreen(hscr,0,hscr->Height+2);
    ScreenToFront(hscr);
    while(hscr->TopEdge > topedge)
	{
	WaitBOVP(hvp);
	if((hscr->TopEdge - topedge) > step)
		MoveScreen(hscr,0,-step);
	else
		MoveScreen(hscr,0,-(hscr->TopEdge - topedge));
	}
    /* Blit HID to VIS */
    BltBitMapRastPort(hrp->BitMap,0,0,
		     vrp,0,0,
		     hscr->Width, hscr->Height,
		     0xC0);
    /* Copy HID colors to VIS */
    /* WARNING - used to get colors right from colortable of hidden viewport */
    setcolors(ilbm[HID],vvp);
    /* bring VIS to front */
    ScreenToFrontReset(vscr);
    }

void fadeout()
    {
    UWORD rgb, newtable[256];
    int k,j,frame;

    j = MIN(256,vvp->ColorMap->Count);

    for(k=0; k<j; k++) newtable[k] = GetRGB4(vvp->ColorMap,k);

    for(frame=0; frame<16; frame++)
	{
	for(k=0; k<j; k++)
	    {
	    rgb = newtable[k];
	    if(rgb & 0xF00)	rgb -= 0x100;
	    if(rgb & 0x0F0)	rgb -= 0x010;
	    if(rgb & 0x00F)	rgb -= 0x001;
	    newtable[k] = rgb;
	    }
	WaitTOF();
	LoadRGB4(vvp,newtable,j);
	LoadRGB4(hvp,newtable,j);
	}
    FadedOut = TRUE;
    }

void fadein(struct ILBMInfo *ilbmpal, BOOL DoUpdate)
    {
    UWORD rgb, newtable[256];
    int k,j,frame;

    FadedOut = FALSE;
    if(DoUpdate)	update();

    j = MIN(256,ilbmpal->ncolors);

    if(j==0)
	{
	printf("no palette for this bg (ilbm $%lx - skipping fadein\n",ilbmpal);
	return;
	}

    for(k=0; k<j; k++) newtable[k] = 0;

    for(frame=0; frame<16; frame++)
	{
	for(k=0; k<j; k++)
	    {
	    rgb = newtable[k];
	    if((rgb & 0xF00) < (ilbmpal->colortable[k] & 0xF00))	rgb += 0x100;
	    if((rgb & 0x0F0) < (ilbmpal->colortable[k] & 0x0F0))	rgb += 0x010;
	    if((rgb & 0x00F) < (ilbmpal->colortable[k] & 0x00F))	rgb += 0x001;
	    newtable[k] = rgb;
	    }
	WaitTOF();
	LoadRGB4(vvp,newtable,j);
	LoadRGB4(hvp,newtable,j);
	}
    setcolors(ilbm[bgnum],vvp);
    setcolors(ilbm[bgnum],hvp);
    }


LONG getbrush(UBYTE *name, int bi)
   {
   struct BitMap *bm;
   UWORD *wp1, *wp2;
   UBYTE *mask;
   ULONG bytes;
   LONG error = 0;
   int n, k;

   /* if this is not the currently loaded brush */
   if((stricmp(name,brunames[bi]))||(!(ilbm[bi]->brbitmap)))
	{
	/* Unload old brush and mask */
	if(ilbm[bi]->brbitmap)	unloadbrush(ilbm[bi]);
	if(brumask[bi])
	    {
	    FreeMem(brumask[bi],brumaskbytes[bi]);
	    brumask[bi] = NULL;
	    }
	/* copy new name and load new brush */
	strcpy(brunames[bi],name);
    	if(error = loadbrush(ilbm[bi],brunames[bi]))
	    {
	    printf("Warning - brush %s load failed\n",brunames[bi]);
	    strcpy(brunames[bi],"");
	    }
	else	/* load successful - make a mask */
	    {
	    bm = ilbm[bi]->brbitmap;
	    bytes = bm->BytesPerRow * bm->Rows;

	    if(mask = AllocMem(bytes,MEMF_CHIP|MEMF_CLEAR))
		{
		brumask[bi] = mask;
		brumaskbytes[bi] = bytes;
		for(n=0; n<bm->Depth; n++)
		    {
		    for(k=0, wp1=(UWORD *)mask, wp2=(UWORD *)bm->Planes[n];
				 	k < (bytes>>1);
					    		k++)
			{
			wp1[k] |= wp2[k];
			}
		    }
		}
	    else(printf("Can't alloc brush mask for %s\n",name));
	    }	
	}
    return(error);
    }


void blitbrush(struct RastPort *rp, SHORT x,SHORT y,
			struct ILBMInfo *ilbm, UBYTE *mask)
    {
    if(!ilbm)
	{
	printf("Slider: have no brush to blit\n");
	return;
	}
    if(mask)
	{
    	BltMaskBitMapRastPort(ilbm->brbitmap,0,0,
    		rp,x,y,
     		ilbm->Bmhd.w, ilbm->Bmhd.h,
     		(ABC|ABNC|ANBC), (PLANEPTR)mask);
	}
    else
	{
    	BltBitMapRastPort(ilbm->brbitmap,0,0,
	    	rp,x,y,
	     	ilbm->Bmhd.w, ilbm->Bmhd.h,
	     	0xC0);
	}
    WaitBlit();
    }


BOOL loadFont(int n, char *name, int size, ULONG dpi, ULONG style)
   {
   struct TextFont *font;

   /*
   printf("Requested font #%ld %s size=%ld style=$%lx dpi=$%08lx\n",
		n,name,size,fontstyles[n],dpi);
   */

   if(n >= NUMFONTS) return(FALSE);

   if(fonts[n])
	{
	CloseFont(fonts[n]);
	fonts[n] = NULL;
	}
   
   ttAttrs[n].tta_Name = name;
   ttAttrs[n].tta_YSize = size;
   ttAttrs[n].tta_Style = style | FSF_TAGGED;
   ttAttrs[n].tta_Flags = fontflags;
   ttAttrs[n].tta_Tags = (struct TagItem *)&fonttags[0];
   if(dpi)
	{
	fonttags[0] = TA_DeviceDPI;
   	fonttags[1] = dpi;
	}
   else
	{
	fonttags[0] = TAG_IGNORE;
   	fonttags[1] = 0L;
	}

   if(!(font=(struct TextFont *)OpenDiskFont(&ttAttrs[n])))
        {
        printf("Can't open font %s size %ld... ",name,size);
	ttAttrs[n].tta_Name = dpi ? defbulfontname : defbitfontname;
        printf("trying %s\n",ttAttrs[n].tta_Name);
	font=(struct TextFont *)OpenDiskFont(&ttAttrs[n]);
	if(!font)	return(FALSE);
        }

   fonts[n] = font;
   return(TRUE);
   }

void setFont(int n)
    {
    if(fonts[n])
	{
	SetFont(hrp,fonts[n]);
	SetSoftStyle(hrp,fontstyles[n],FSF_ITALIC|FSF_BOLD);
	fontsize = fonts[n]->tf_YSize;
	baseline = fonts[n]->tf_Baseline;
	}
    else printf("font is null\n");
    }

void doBox(UWORD bx, UWORD by, UWORD bw, UWORD bh)
    {
    SetDrMd(hrp,JAM1);

    /* the shadow, if any */
    if((shadx)||(shady)||(shadw)||(shadh))
	{
    	SetAPen(hrp,shadpen);
    	RectFill(hrp,bx+shadx,by+shady,
			(bx+shadx)+bw+shadw-1,(by+shady)+bh+shadh-1);
	}

    if((shadx2)||(shady2)||(shadw2)||(shadh2))
	{
    	SetAPen(hrp,shadpen2);
    	RectFill(hrp,bx+shadx2,by+shady2,
			(bx+shadx2)+bw+shadw2-1,(by+shady2)+bh+shadh2-1);
	}

    /* the box */
    SetAPen(hrp,pen);
    RectFill(hrp,bx,by,bx+bw,by+bh);
    }




int showIt(void)
   {
   UBYTE  *buf;
   int rLen = 1; 

   ty = tmargin;

   buf = &lbuf[0];

   BgDone  = FALSE;
   NotesOn = FALSE;
   SlidUp  = FALSE;

   if(skipto)
	{
   	while((rLen>0)&&(slidenum)&&(slidenum < skipto))
	    {
	    rLen = getLine(buf);
   	    while((rLen >= 0)&&(strnicmp(buf,SLIDE_END,strlen(SLIDE_END))))
	    	{
	    	rLen = getLine(buf);
	    	}
	    slidenum++;
	    }
	}

   /* Parse (macros and inits etc.) until start of a slide */
   while(((rLen=getLine(buf))>=0)&&(strnicmp(buf,SLIDE_START,strlen(SLIDE_START))))
	{
	if(*buf == '.')	parseBuf(buf);
	}
   if(rLen < 0) return(0);

   doBg(bgnum);

   rLen = getLine(buf);

   /* Until end of slide (we already have a line in buf) */
   while((rLen >= 0)&&(strnicmp(buf,SLIDE_END,strlen(SLIDE_END))))
	{
   	parseBuf(buf);
	rLen = getLine(buf);
	}
   NotesOn = FALSE;
   return(rLen > 0 ? rLen : 0);
   }


void doText(UBYTE *buf)
    {
    int k; 
    SHORT x, y, w, h, l, bw, bx=0, by=0;
    BOOL  InvisBullet;

    if((!NotesOn)&&((*buf == '-')||(*buf == '='))&&(buf[1] == '\0'))
	{
	k = (*buf == '-') ? 2 : 1;
	ty += ((fontsize + vertspace) >> k);
	} 	
    else if(!NotesOn)				/* Display Text */
	{
	/* Character spacing */
	vrp->TxSpacing = charspace;
	hrp->TxSpacing = charspace;

	/* Top and left edge positioning */
	ty += (fontsize + vertspace);
	tx = lmargin;
	bw = 0;
	InvisBullet = FALSE;

	if(*buf == '@')
	    {
	    bullet++;
	    buf++;
	    if(*buf == '@')	/* @@ Invisible bullet */
		{
		InvisBullet = TRUE;
		buf++;
		}
	    }


	/* Centering and Left Edge Positioning */
	if(bullet)
	    {
	    bullet--;
	    if(BulletBrush)
		{
		bw = ilbm[BUL]->Bmhd.w;
		bx = tx + bulletxoffs;
/* This would be a baseline seated bullet but we'll use centered
		by = ty + bulletyoffs + 1 - ilbm[BUL]->Bmhd.h;
 */
		by = (ty - baseline) + bulletyoffs +
				((fontsize - ilbm[BUL]->Bmhd.h)>>1);
		tx = tx + bw + TextLength(hrp,"t",1);
		}
	    else
		{
		bw = TextLength(hrp,bulletstring,strlen(bulletstring));
		bx = tx + bulletxoffs;
		by = ty + bulletyoffs;
		tx = tx + bw + TextLength(hrp,"t",1);
		}
	    }
	else if(center)
	    {
	    center--;
	    tx = ((hwin->Width)-TextLength(hrp,buf,strlen(buf))) >> 1;
	    }

	/* Bullets...*/
	if((bw)&&(BulletBrush))
	    {
    	    if(!InvisBullet) blitbrush(hrp, bx, by, ilbm[BUL], brumask[BUL]);
	    }
	else if((bw)&&(!InvisBullet))
	    {
	    /* Bullet shadow */
    	    if((shadx)||(shady)||(shadw)||(shadh))
		{
		/* The bullet shadow */
                SetAPen(hrp,shadpen);   /* the shadow color */
                for(x=bx+shadx,w=0; w<shadw; w++)
                    {
                    for(y=by+shady,h=0; h<shadh; h++)
                        {
                        if((x!=bx)||(y!=by))
                            {
                            Move(hrp,x,y);
                            Text(hrp,bulletstring,strlen(bulletstring));
                            }
                        y++;
                        }
                    x++;
                    }
		}

	    /* Bullet shadow 2 */
    	    if((shadx2)||(shady2)||(shadw2)||(shadh2))
		{
                SetAPen(hrp,shadpen2);   /* the shadow color */
                for(x=bx+shadx2,w=0; w<shadw2; w++)
                    {
                    for(y=by+shady2,h=0; h<shadh2; h++)
                        {
                        if((x!=bx)||(y!=by))
                            {
                            Move(hrp,x,y);
                            Text(hrp,bulletstring,strlen(bulletstring));
                            }
                        y++;
                        }
                    x++;
                    }
		}

	    /* Bullet itself */
	    SetAPen(hrp, bulletpen);
	    Move(hrp,bx,by);
	    Text(hrp,bulletstring,strlen(bulletstring));	
	    }

	/* Text Shadow */
        l = strlen(buf);

	if((shadx)||(shady)||(shadw)||(shadh))
	    {
            SetAPen(hrp,shadpen);       /* the shadow color */
            for(x=tx+shadx,w=0; w<shadw; w++)
                {
                for(y=ty+shady,h=0; h<shadh; h++)
                    {
                    if((x!=tx)||(y!=ty))
                        {
                        Move(hrp,x,y);
                        Text(hrp,buf,l);
                        }
                    y++;
                    }
                x++;
                }
	    }

	/* Text Shadow2 */
    	if((shadx2)||(shady2)||(shadw2)||(shadh2))
	    {
            SetAPen(hrp,shadpen2);       /* the shadow2 color */
            for(x=tx+shadx2,w=0; w<shadw2; w++)
                {
                for(y=ty+shady2,h=0; h<shadh2; h++)
                    {
                    if((x!=tx)||(y!=ty))
                        {
                        Move(hrp,x,y);
                        Text(hrp,buf,l);
                        }
                    y++;
                    }
                x++;
                }
	    }

	/* Text itself */
	SetAPen(hrp, pen);
	Move(hrp,tx,ty);
	Text(hrp,buf,l);	
   	}
    }


void doBg(int n)
   {
   SetAPen(hrp,0);
   SetDrMd(hrp,JAM1);

   switch(n)
	{
	/* HID used for the normal background 1 */
	/* VIS used for the alternate background 2 */
	case HID: case VIS:
	    SetRast(hrp,bgpen);
	    if(ilbm[n]->brbitmap)
	    	{
	   	BltBitMapRastPort(ilbm[n]->brbitmap,0,0,
		     hrp,0,0,
		     ilbm[n]->Bmhd.w, ilbm[n]->Bmhd.h,
		     0xC0);
	    	}
	    break;

	default:
	    SetRast(hrp,bgpen);
	    break;
	}
    BgDone = TRUE;
    }

void skipSlide(UBYTE *buf)
   {
   while((getLine(buf) >= 0)&&(strnicmp(buf,SLIDE_END,strlen(SLIDE_END))));
   buf[0] = '\0';
   }


/* Get a line - return -1L for end of file, length of line otherwise */
int getLine(UBYTE *buf)
    {
    if((lindex>=linecnt)||(!(linearray[lindex])))  return(-1L);
    else
	{
	strcpy(buf,linearray[lindex]);
	lindex++;
	return((int)(strlen(buf)));
	}
    }


/* resets screen to original top/left position and brings to front */
void ScreenToFrontReset(struct Screen *scr)
   {
   WORD topdelta=0, leftdelta=0;

   if(scr->LeftEdge != leftedge)	leftdelta=leftedge-scr->LeftEdge;
   if(scr->TopEdge  != topedge)		topdelta=topedge-scr->TopEdge;
   if(topdelta || leftdelta)		MoveScreen(scr,leftdelta,topdelta);

   ScreenToFront(scr);
   }

void bye(UBYTE *s,int error)
   {
   if((*s)&&(!FromWb)) printf(s);
   cleanup();
   exit(error);
   }

UWORD allblack[32] = {0};

void cleanup()
   {
   int k;

   if(vvp) LoadRGB4(vvp,allblack,32);
   if(hvp) LoadRGB4(hvp,allblack,32);

   if(hwin)	RemoveUserPort(hwin);	/* shared with vwin */

   if(ilbm[DEF])
	{
	for(k=1; k<ILBMCNT; k++)
	    {
   	    if(ilbm[k])
		{
   		if(ilbm[k]->brbitmap)		unloadbrush(ilbm[k]);
   		if(ilbm[k]->scr)		closedisplay(ilbm[k]);
   		if(ilbm[k]->ParseInfo.iff) 	FreeIFF(ilbm[k]->ParseInfo.iff);
		}
	    if(brumask[k])	FreeMem(brumask[k],brumaskbytes[k]);
	    }
   	FreeMem(ilbm[0],ILBMCNT * sizeof(struct ILBMInfo));
	}

   if(pdata)	 FreeMem(pdata,PDATASZ);

   if(linetext)  FreeMem(linetext,ltsz);
   if(linearray) FreeMem(linearray,lasz);
   if(smacs)	 FreeMem(smacs,smsz);

   for(k=0; k<NUMFONTS; k++)
	{
	if(fonts[k])	CloseFont(fonts[k]);
	}

   if(DiskfontBase)  CloseLibrary(DiskfontBase);
   if(GfxBase) 	     CloseLibrary(GfxBase);
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   if(IFFParseBase)  CloseLibrary(IFFParseBase);
   }


/* passed name of slides file, sets up null termed linetext and linearray. 
 * linecount returned (returns 0 for failure)
 */
ULONG getslidefile(UBYTE *filename)
{
LONG file,i;
ULONG lcnt=0, ll, mm;

    if(file = Open(filename,MODE_OLDFILE))
	{
	if(ContainsBinary(file))	return(0);
	Seek(file,0,OFFSET_END);
	ltsz = Seek(file,0,OFFSET_BEGINNING);

	/* Read in the whole file */
	if(linetext = AllocMem(ltsz,MEMF_PUBLIC))
	    {
	    Read(file,linetext,ltsz);
	    }
	Close(file);
	if(!linetext) return(0L);

	/* Null terminate and count the lines */
	for(i=0; i<ltsz; i++) if(linetext[i]=='\n')
	    {
	    lcnt++;
	    linetext[i] = '\0';
	    }

	/* allocate an array of pointers to the lines */
	lasz = (lcnt+1) << 2;
	if(!(linearray = (UBYTE **)AllocMem(lasz, MEMF_PUBLIC|MEMF_CLEAR)))
	    {
	    FreeMem(linetext,ltsz);
	    return(0L);
	    }

	/* initialize the array while counting the macros */
	for(ll=0,i=0; i<ltsz; i++)
	    {
	    linearray[ll] = &linetext[i];
	    if(!(strnicmp(linearray[ll],SLIDE_MACRO,strlen(SLIDE_MACRO))))
		mcnt++;
	    while((i<ltsz)&&(linetext[i])) i++;
	    ll++;
	    }
	if(ll > lcnt) ll=lcnt;
	linearray[ll] = NULL;

        smsz = (mcnt + 1) << 2;
	if(!(smacs = (ULONG *)AllocMem(smsz, MEMF_PUBLIC|MEMF_CLEAR)))
	    {
	    FreeMem(linetext,ltsz);
	    FreeMem(linearray,lasz);
	    return(0L);
	    }

	for(mm=0,ll=0; (ll<lcnt) && (mm<mcnt); ll++)
	    {
	    if(!(strnicmp(linearray[ll],SLIDE_MACRO,strlen(SLIDE_MACRO))))
		{
		smacs[mm] = ll;
		mm++;
		}
	    }
	smacs[mcnt] = ENDMACROS;	/* terminator */
	}
    return(lcnt);
}

void InstallUserPort(struct Window *win, struct MsgPort *mp, ULONG idcmpflags)
   {
   if(!win)	return;
   Forbid();
   win->UserPort = mp;
   ModifyIDCMP(win,idcmpflags);
   Permit();
   }

void RemoveUserPort(struct Window *win)
   {
   Forbid();

   /* Send back any unprocessed messages for this window */
   StripIntuiMessages(win,win->UserPort);

   /* Null UserPort so Intuition won't free it */
   win->UserPort = NULL;

   /* Tell Intuition to stop sending more messages */
   ModifyIDCMP(win,0);

   /* Turn tasking back on */
   Permit();
   }

void StripIntuiMessages(struct Window *win, struct MsgPort *mp)
   {
   struct IntuiMessage *msg, *succ;

   msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;

   while(succ = (struct IntuiMessage *)msg->ExecMessage.mn_Node.ln_Succ)
      {
      if(msg->IDCMPWindow == win)
         {
         /* Intuition is about to rudely free this message.
          * Make sure that we have politely sent it back.
          */
         Remove(msg);
         ReplyMsg(msg);
         }
      msg = succ;
      }
   }

   

void getWbOpts(wbArg)
struct WBArg  *wbArg;
    {
    struct DiskObject *diskobj;
    char **toolarray;
    char *s;
    ULONG u;

    if((IconBase = OpenLibrary("icon.library", 0)))
      	{
      	/* Get ToolTypes from Display.info */
      	diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name);
      	if(diskobj)
     	    {
            toolarray = (char **)diskobj->do_ToolTypes;

    	    Auto = ttTrue(toolarray,"AUTO");
	    NoBg = ttTrue(toolarray,"NOBG");
	    if(ttTrue(toolarray,"LACEBG"))
		{
	    	NoBg = TRUE;
	    	ns.ViewModes |= LACE;
	    	if(ns.Height == YHIGH)	ns.Height = YHIGH << 1;
	    	sheight = ns.Height;
		}

    	    if(s=(char *)FindToolType(toolarray,"BGNAME"))
        	{
		strcpy(bgnamebuf,s);
   		names[1] = bgnamebuf;
		}

    	    if(s=(char *)FindToolType(toolarray,"SKIP"))
        	{
		skipto=atoi(s);
		}

    	    if(s=(char *)FindToolType(toolarray,"HEIGHT"))
        	{
		if(u = atoi(s))
		    {
		    sheight = ns.Height = u;
	    	    if(sheight >= 400)	ns.ViewModes |= LACE;
		    }
		else printf("invalid screen height: %ld\n",u);
		}

    	    if(s=(char *)FindToolType(toolarray,"WIDTH"))
        	{
		if(u = atoi(s))
		    {
		    swidth = ns.Width = u;
	    	    if(swidth >= 640)	ns.ViewModes |= HIRES;
	    	    else		ns.ViewModes &= (~HIRES);
		    }
		else printf("invalid screen width: %ld\n",u);
		}

            FreeDiskObject(diskobj);
            }
      	CloseLibrary(IconBase);
      	}
    }

BOOL ttTrue(char **toolarray, UBYTE *keyword)
{
UBYTE *s;

    if(s=(char *)FindToolType(toolarray,keyword))
        {
        if(!(stricmp(s,"TRUE")))  return(TRUE);
        }
    return(FALSE);
}

int getval(char *s)
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }

/*
 * Synchronous external command (wait for return)
 * Uses your Input/Output unless you supply other handle
 * Result will be return code of the command, unless the System() call
 * itself fails, in which case the result will be -1
 */
LONG syncsys(UBYTE *command, BPTR other)
    {
    extern struct Library *SysBase; 
    struct TagItem stags[4];

    if(SysBase->lib_Version < 36)	return(VERSIONFAIL);
 
    stags[0].ti_Tag = SYS_Input;
    stags[0].ti_Data = other ? other : Input();
    stags[1].ti_Tag = SYS_Output;
    stags[1].ti_Data = other ? NULL: Output();
    stags[3].ti_Tag = TAG_DONE;
    return(System(command, stags));
    }


/*
 * Asynchronous external command started with its own autocon Input/Output
 * This routine shows use of the SYS_UserShell tag as well.
 * Result will only reflect whether System() call itself succeeded.
 * If System() call fails, result will be -1L
 * We are using -2L as result if our Open of CON: fails
 */
UBYTE *autocon="CON:0/40/640/150/Slider/auto/close";
LONG asyncsys(UBYTE *command)
    {
    extern struct Library *SysBase; 
    struct TagItem stags[5];
    LONG result = WINDOWFAIL;
    BPTR file;

    if(SysBase->lib_Version < 36)	return(VERSIONFAIL);

    if(file = Open(autocon, MODE_OLDFILE))
	{
	stags[0].ti_Tag = SYS_Input;
	stags[0].ti_Data = file;
	stags[1].ti_Tag = SYS_Output;
	stags[1].ti_Data = NULL;
	stags[2].ti_Tag = SYS_Asynch;
	stags[2].ti_Data = TRUE;
	stags[3].ti_Tag = SYS_UserShell;
	stags[3].ti_Data = TRUE;
	stags[4].ti_Tag = TAG_DONE;
        result = System(command, stags);
	if(result == -1L)
	    {
	    Close(file);
	    }
	}
    return(result);
    }

/*------------------------------------------------------------------------*/





