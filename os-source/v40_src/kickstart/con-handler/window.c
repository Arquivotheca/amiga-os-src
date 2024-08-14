#include <exec/types.h>
#include <internal/librarytags.h>
#include <graphics/gfxbase.h>

#include "con-handler.h"

/* Private ROM routine... */
struct Library *TaggedOpenLibrary(LONG);
#pragma syscall TaggedOpenLibrary 32a 001

static UBYTE * __regargs winParse(struct Global *, UBYTE *, WORD *, UBYTE);

struct Window * __regargs ParseParam(gv,dev,unit_no,flags)
struct Global *gv;
UBYTE *dev;
LONG unit_no;
LONG flags;
{

    struct IntuitionBase *IntuitionBase=(struct IntuitionBase *)gv->IntuitionBase;
    struct Rectangle rect = {10,10,80,40};
    struct Window *awindow=0;
    struct Screen *scr,*oscr,*fscr,*screen;
    struct GfxBase *GfxBase;
    LONG unit;
    LONG id;
    LONG alt=0;
    struct Process *process;
    int len,ac,flag=0;
    /* load pointers from gv */
    UBYTE *param=gv->titlemem;	/* use for copying title to buffer */
    UBYTE *ostring=gv->stringmem;
    UBYTE *nbuffer=gv->namemem;


    struct IOStdReq *iob=    (struct IOStdReq *)IOB;
    struct IOStdReq *iobo =  (struct IOStdReq *)IOBO;


    WORD win[4]={0};
    UBYTE *hard_parm[15]={"WAIT","AUTO","CLOSE","NOCLOSE","SMART","SIMPLE",
	"INACTIVE","BACKDROP","NOBORDER","NOSIZE","NODRAG","NODEPTH","WINDOW",
	"SCREEN","ALT"};

    // Don't move entries in this array.  Code modifies these by index #.
    LONG wflags = WINDOWDEPTH|WINDOWSIZING|WINDOWDRAG|ACTIVATE|SIMPLE_REFRESH;
    struct TagItem tags[] = {
	{WA_AutoAdjust,TRUE},				// 0
	{WA_Zoom, NULL},				// 1
	{WA_PubScreenFallBack,TRUE},			// 2
	{TAG_IGNORE,NULL},				// 3
	{WA_MinWidth,80},	// from anw in con.c	// 4
	{WA_MinHeight,50},	// ditto		// 5
	{WA_Left,0},					// 6
	{WA_Top,0},					// 7
	{WA_Width,640},					// 8
	{WA_Height,100},				// 9
	{WA_MaxWidth,-1},				// 10
	{WA_MaxHeight,-1},				// 11
	{WA_Flags,0},					// 12
	{WA_Title,(LONG) "AmigaDOS"},			// 13
	TAG_DONE,NULL};

if((LONG)window > 0)awindow=window; /* then we are opening in an existing window */
else if (dev) {
	unit=unit_no;
	awindow=(struct Window *)2;
}
else { /* we need a new window */
      /* ostring is initialized to point to source openstring buffer */
  while(*ostring)if(*ostring++ == ':')break; /* find : */

  if(*ostring) {	/* yes, there are window parameters */
        ostring=winParse(gv,ostring,(WORD *)&win,'/');

      	tags[6].ti_Data = win[0];	// left
      	tags[7].ti_Data = win[1];	// top
      	tags[8].ti_Data = win[2];	// width
      	tags[9].ti_Data = win[3];	// height

        /* copy title string */
	// make sure we don't copy too much!
	while(*ostring && (*ostring != '/') &&
	       (param - gv->titlemem) < sizeof(gv->titlemem)-1)
	{
		*param++ = *ostring++;
	}
	*param=0; /* make sure its null terminated */

      	param=ostring; /* now set param to source open string */

        while(*param) {
	    if(*param++ == '/') {

		// This is REALLY inefficient... REJ
		for(ac=0; ac<15; ac++) {
		  len=strlen(hard_parm[ac]);
		  if(!Strnicmp(hard_parm[ac],param,len)) {
		    param += len;
		    if(*param==' ')param++; /* skip a leading space */
		    break;
		  }
		}
		switch (ac) {
		    case 0: /* wait */
		    gv->waitflag = TRUE;
		    break;

		    case 1:	/* auto */
		    gv->AUTOflag = 1;
		    /* fall through to ... */

		    case 2: /* close */
		    gv->CLOSEflag=TRUE;
	 	    wflags |= WINDOWCLOSE;
		    break;

		    case 3: /* noclose */
	 	    wflags &= ~WINDOWCLOSE;
		    if(gv->AUTOflag)gv->AUTOflag++;
		    break;

		    case 4: /* smart */
		    wflags |= SMART_REFRESH|NOCAREREFRESH;
		    wflags &= ~SIMPLE_REFRESH;
		    break;

		    case 5: /* simple */
		    wflags |= SIMPLE_REFRESH;
		    wflags &= ~(SMART_REFRESH|NOCAREREFRESH);
		    break;

		    case 6: /* inactive */
		    wflags &= ~ACTIVATE;
		    break;

		    case 7: /* backdrop */
		    wflags |= BACKDROP;
		    break;

		    case 8: /* noborder */
		    wflags |= BORDERLESS;
		    gv->titlemem[0] = 0; /* no title */
		    break;

		    case 9: /* nosize */
		    wflags &= ~WINDOWSIZING;
		    (tags[1]).ti_Tag=TAG_IGNORE;
		    break;

		    case 10: /* nodrag */
		    wflags &= ~WINDOWDRAG;
		    break;

		    case 11: /* nodepth */
		    wflags &= ~WINDOWDEPTH;
		    break;

		    case 12: /* window */
		       if(*param=='0')param++;
		       if((*param&0x5f)=='X')param++;
		       stch_l(param,(long *)&awindow);
			/* cheesy test, but shorter than a scan through */
			/* window list, and bytes are short */
		       if(((ULONG)awindow&3) || (!awindow) ||
			 (!awindow->WScreen) || ((ULONG)(awindow->WScreen)&3))
		       		awindow=0;
		       break;

		    case 13: /* screen */
		        if(*param == '*') { /* use first screen */
			    /* attempt to find oscr by using NextScreen */
			    oscr=(struct Screen *)IntuitionBase->FirstScreen;
			    scr=LockPubScreen(NULL); /* start with WB */
			    fscr=scr; /* save initial value for end detect */
			    while(scr) {
				if(!NextPubScreen(scr,nbuffer))break;
				UnlockPubScreen(NULL,scr);
			        scr=LockPubScreen(nbuffer);
				if(scr == oscr) break; /* found front! */
				if(scr == fscr) break; /* no screens */
				}
			    if(scr) {
				UnlockPubScreen(NULL,scr);
				if(scr==oscr) {
			            (tags[3]).ti_Data=(ULONG)nbuffer;
		                    (tags[3]).ti_Tag=WA_PubScreenName;
				}
			    }
			}
			else {
			    (tags[3]).ti_Data=(ULONG)param;
		            (tags[3]).ti_Tag=WA_PubScreenName;
			}
		        break;

		    case 14: /* alt */
		        alt=TRUE;
		        param=winParse(gv,param,(WORD *)&win,',');
		        if(*param)param--;
		        break;

		default:;
		}
	    }
	}
      	tags[13].ti_Data = (LONG) gv->titlemem;
      }
   tags[12].ti_Data = wflags;

   /* check if this was just the parse to find the autoflag */
   if((LONG)window == (-1))return((struct Window *)0);

/* if WINDOW is set here, we got a window pointer in the title string */
    if(!awindow) {
	// Can't fail
	GfxBase=(struct GfxBase *)TaggedOpenLibrary(OLTAG_GRAPHICS);

	// This is slightly wierd.  It LPS's tags[3].ti_Data even if it
	// was never set.  Luckily, NULL is a valid input for LPS.
 	if(!(screen=LockPubScreen((char *)(tags[3]).ti_Data)))
	        screen=LockPubScreen(NULL); /* failed, so go to default */
					    /* this should be the same */
					    /* effect as the fallback */
					    /* tag for openwindow */
	if(screen) {
		LONG temp;

		id=GetVPModeID(&screen->ViewPort);
		if(id != INVALID_ID)QueryOverscan(id,&rect,OSCAN_TEXT);

		// set window minimum size relative to Titlebar height/etc
		// tags already set up with defaults that assume topaz 8
		tags[4].ti_Data += screen->WBorLeft + 
				   screen->WBorRight - (4+4);
	        temp = screen->WBorTop+1 + screen->Font->ta_YSize +
		       screen->WBorBottom;
		tags[5].ti_Data += temp - (2+2+8+1);

		// Make sure we have space for one line...
		if (tags[5].ti_Data < temp + screen->Font->ta_YSize)
			tags[5].ti_Data = temp + screen->Font->ta_YSize;

/*kprintf("minwidth = %ld, minheight = %ld\n",tags[4].ti_Data,tags[5].ti_Data);*/
	}

// FIX!!  The 10 width should be font sensitive!
        if(((LONG) tags[6].ti_Data) < 0)
			tags[6].ti_Data = rect.MinX;	// left
        if(((LONG) tags[7].ti_Data) < 0)
			tags[7].ti_Data = rect.MinY;	// top
        if(((LONG) tags[8].ti_Data) < 10)
			tags[8].ti_Data = rect.MaxX+1;	// width
	if(((LONG) tags[9].ti_Data) < 0)
			tags[9].ti_Data = rect.MaxY+1;	// height
        else if(((LONG)tags[9].ti_Data) < tags[5].ti_Data)
			tags[9].ti_Data = tags[5].ti_Data;  // height

        tags[1].ti_Data=(ULONG)win;
	if(!alt) { /* no ALT, so take full screen as default */
	    win[0]=rect.MinX;
	    win[1]=rect.MinY;
	    win[2]=rect.MaxX+1;
	    win[3]=rect.MaxY+1;
	}

	awindow=(struct Window *)OpenWindowTagList(NULL, tags);
	if (awindow)
	{
		// change to a fixed-width font if this is proportional
		// DefaultFont is guaranteed to be fixed-width.
		if (awindow->IFont->tf_Flags & FPF_PROPORTIONAL)
			SetFont(awindow->RPort,GfxBase->DefaultFont);
	}
	if(screen)UnlockPubScreen(NULL,screen);
	CloseLibrary(GfxBase);
    }
}

if (awindow) { /* we have a window, now for the console */
        memset((char *)iob,0,sizeof(struct IOStdReq));
	iob->io_Data = (APTR)awindow;
	process = THISPROC;
	iob->io_Message.mn_ReplyPort = &process->pr_MsgPort;

	if(!dev) {
	    if(wflags & SMART_REFRESH)unit=0;
	    else unit=3;
	    dev="console.device";
	    flag=TRUE;
	}
      	if (OpenDevice(dev,unit,(struct IORequest *)iob,flags)) {
		if(((LONG)awindow)>2)CloseWindow(awindow);
		awindow = 0;
	}
	else {
	    *iobo = *iob;
	    if(flag)gv->cu=(struct ConUnit *)iob->io_Unit;
	   /*  Fix color used, and NL fix unless raw */
	   /* and request close events */
   	    fixbuf (iobo, "\x9B" "31m" );
            if (!raw) {
		fixbuf (iobo, "\x9B" "20h");
		if(gv->CLOSEflag)fixbuf(iobo, "\x9B" "11{");
  	    }
	}
   }
   return(awindow);
}

static
UBYTE * __regargs winParse(gv,string,win,delimiter)
struct Global *gv;
UBYTE *string;
WORD *win;
UBYTE delimiter;
{
struct DosLibrary *DOSBase=(struct DosLibrary *)gv->DOSBase;
LONG i,len,sum;

    for (i = 0 ; i <= 3 ; i++) {
	if(!(*string))break;	/* no more */
	win[i] = 0;
	if ((len = StrToLong(string,&sum)) != -1)
	{
		// got good number
		string += len;
		if (!*string || *string == delimiter || *string == '/')
			win[i] = sum;
		// else win[i] = 0;
	}
	/* skip to delimiter or '/' */
	while (*string && *string != delimiter && *string != '/')
		string++;
	/* must point past delimiter or '/' */
	if (*string)
		string++;
    }
    return(string);
}

void __regargs CloseCon(gv,wind,iob)
struct Global *gv;
struct Window *wind;
struct IOStdReq *iob;
{

struct Library *IntuitionBase= gv->IntuitionBase;

   if(wind)CloseDevice ((struct IORequest *)iob);
   if(wind> (struct Window *)2)CloseWindow(wind);
}
