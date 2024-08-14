/*
 * Display - Carolyn Scheppner   CBM
 *
 *   Read an ILBM file and display as a screen/window until closed.
 *   Simulated close gadget in upper left corner of window.
 *   Clicking below title bar area toggles screen bar for dragging.
 *   Handles normal and HAM ILBM's
 *   Now has options for backscreen, timer, cycling, printing
 *
 *   Options:
 *        
 *          m     mousebuttons advance screens
 *          l     loop back to first after last
 *          b     screen should stay behind other screens and inactive
 *          c     cycle colors (currently not implemented)
 *          p     autodump to printer (press CTRL-P to manually dump) 
 *          e     default 6 planes to extra-halfbrite
 *	    n     notransparent borders
 *	    a	  autoscroll (if possible)
 *	    v     full video dclip
 *          t=n   where n = display time in seconds (without or after dump)
 *
 *  By Carolyn Scheppner   CBM
 *
 * Originally Based on ShowILBM.c, readpict.c    1/86
 *  By Jerry Morrison, Steve Shaw, and Steve Hayes, Electronic Arts.
 *  This software is in the public domain.
 *
 * Modified 03/20/90 - rewritten for 1.4 modes, and iffparse.library
 *	support added using code by Leo Schwab and Stu Ferguson
 *
 * >>NOTE<<: This example must be linked with additional modules
 *           See lmkfile
 *
 * The display portion is specific to the Commodore Amiga computer.
 *
 * 36_15 mods:  shorten title, look in :libs/ for iffparse, no cycle comments 
 * 36_16 mods:  add filelist capability, wmain.obj link
 * 36_17 mods:  fix timer, filelist opt capability, continue if bad pic
 * 36_18 mods:  add bordernotrans to screen.c
 * 36_19 mods:  fix brush loading
 * 36_20 mods:  fix handling of individual picture options
 * 36_21 mods:  only mask CAMG on old-style opens, Forbid around DeleteTask
 * 36_22 mods:  add a6 preserve/restore to HookEntry.asm
 * 36_23 mods:  do ModeID screendump, center in smallest Prefs rect
 * 36_24 mods:  make screen size bmhd->w if RowBytes(w) >= RowBytes(pagewidth)
 * 36_25 mods:  screen.c kludge for DPaint brush CAMG's, bumprev, no "*" in wmain
 * 36_26 mods:  kludge moved to ilbm_r.c where CAMG is read in
 * 36_27 mods:  added error checking on Open to wmain.c
 * 37_1  mods:  closeifile if not an ILBM
 * 37_2  mods:  add autoscroll option
 * 37_3  mods:  fix enforcer hit in parse.c contextis() for non-ILBM
 * 37_4  mods:  shorten help line, re-add check for iffparse in :libs/
 * 37_5  mods:  don't try to open screen > 8 bitplanes, new startup code
 * 37_6  mods:  ilbm_r screen bad CAMG bits, display don't open Asl
 * 37_7  mods:  link with createtask mod with proper AllocEntry error check
 * 38_1  mods:  localized modules and display, opt mm allows clickaway of
 *		single picture, added pub screen support to modules
 *		(not currently used by display, print warning when
 *		ILBM is inside a complex file.
 * 38_2  mods:  use special iffpstrings files for one catalog (WB release)
 * 38_3  mods:  hardcode locale string ID's, use function, add toodeep msg
 * 38_4  mods:  add printer trouble string, space/return backspace/b moves
 * 38_5  mods:  larger error output window in wmain.c (140 high)
 * 38_6  mods:  was always returning 1; fix to return 0 or RETURN_WARN
 * 38_7  mods:  link with new screen.c (version checks before PubScreenStatus)
 * 38_8  mods:  link with new iffpstrings.c (fixed IFFerr tests and nullstring)
 * 38_9  mods:  new getdisplay.c - uses MIN(ncolors,vp->ColorMap->Count)
 *              and special screen.c with pubscreen handling ifdef'd out
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <devices/printer.h>

#include <graphics/text.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <graphics/gfxmacros.h>

#include <intuition/screens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/iffparse_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#include "display_rev.h"

/*
#define SHAREDCAT
*/

/* Ignore this - this is Commodore stuff for possible combined catalog files */
#ifdef SHAREDCAT
#define CATALOGNAME	"sys/utilities.catalog"
#define INCLUDENAME	<localestr/utilities.h>
#else
#define CATALOGNAME	"display.catalog"
#define INCLUDENAME	"iffp/iffpstringids.h"
#endif


/* Locale stuff */
#define  DISPLAY
#define  STRINGARRAY
#include INCLUDENAME

/* string macro */
#define S(i) GetString(i)

/* For reference
struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};
*/

#include "iffp/ilbmapp.h"

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif

/* Protos for functions in this module */
LONG preload(struct ILBMInfo *, UBYTE *);
LONG show(struct ILBMInfo *);
void enddisplay(struct ILBMInfo *);
LONG handlemsgs(struct ILBMInfo *);
LONG autoshow(struct ILBMInfo *, UBYTE *);
void getToolOpts(struct WBArg *);
void getProjectOpts(struct WBArg *, struct ILBMInfo *);
void getToolOrListOpts(char **toolarray);
void getOptsFromString(UBYTE *opts, struct ILBMInfo *ilbm);
BOOL ttTrue(char **, UBYTE *);
void getCliOpts(int,char **,int);
LONG chkimsgs(struct ILBMInfo *);
BOOL strEqu(UBYTE *, UBYTE *); 
void usage(void);
void cleanup(void);
void bye(UBYTE *,int);
void subTaskRtn(void);
void doCycle(void);
ULONG getfilelist(UBYTE *fromfile);


#define NEXTPIC 	1
#define PREVPIC		2
#define ENDSHOW		4
/* #define NOFILE	5 */

/* for special startup  (may be deleted for c.o) */
BOOL UseMyWbCon = FALSE;
UBYTE  *MyWbCon = "";

/* general usage pointers */
struct Library  *LocaleBase = NULL;
struct Library	*GfxBase = NULL;
struct Library	*IntuitionBase = NULL;
struct Library	*IconBase = NULL;
struct Library	*IFFParseBase = NULL;

/* For WorkBench startup */    
BPTR startLock = NULL;

/* Other globals */
struct Task *mainTask;
BOOL  FromWb;

/* globals options
 * Note - MMouse is if they ask for Mouse twice. Overrides 1-pic turnoff.
 */
BOOL  Print=FALSE,Timer=FALSE,Back=FALSE,Mouse=FALSE,MMouse=FALSE,Loop=FALSE;

/* individual options */
BOOL  EHB=FALSE, Cycle=FALSE, Autoscroll=FALSE, Video=FALSE, Notransb=FALSE; 

/* event flags */
BOOL  TimerOn = FALSE, CycleOn = FALSE, PrepareToDie = FALSE;

/* project is filelist */
BOOL  FileList = FALSE;

LONG  dTime, dTimer, filecount=0L, retcode = RETURN_OK;

ULONG tSig;
BYTE  tSigNum = -1;

UBYTE *fromtext = NULL;
UBYTE **fromarray = NULL;
UBYTE **fromopts = NULL;
ULONG ftsz = NULL, fasz = NULL, fosz=NULL;


/* For alloc to define new pointer */
#define PDATASZ 12
UWORD   *pdata;

void subTaskRtn(void);
char *subTaskName = VERS "SubTask";
struct Task *subTask;

/* strings */
UBYTE ver[] = VERSTAG;
UBYTE Copyright[] = VERS " Copyright (c) 1990-1991 Commodore-Amiga, Inc.";

/* The type of window to display in */
struct   NewWindow      mynw = {
   0, 0,                                  /* LeftEdge and TopEdge */
   0, 0,                                  /* Width and Height */
   -1, -1,                                /* DetailPen and BlockPen */
   MOUSEBUTTONS|VANILLAKEY,               /* IDCMP Flags */
   BACKDROP|BORDERLESS|RMBTRAP,           /* Flags */
   NULL, NULL,                            /* Gadget and Image pointers */
   NULL,                                  /* Title string */
   NULL,                                  /* Put Screen ptr here */
   NULL,                                  /* SuperBitMap pointer */
   0, 0,                                  /* MinWidth and MinHeight */
   0, 0,                                  /* MaxWidth and MaxHeight */
   CUSTOMSCREEN,                          /* Type of window */
   };


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

/* ILBMInfo User Flags */
#define IFFPUF_CYCLE	(1L<<0)


#define DEFINFO 2
#define UICOUNT 3
struct ILBMInfo *ilbms[UICOUNT] = { NULL };

struct IFFHandle *iffs[2] = { NULL };

/*
 *  MAIN
 */

void main(int argc, char **argv)
    {
    struct WBStartup *WBenchMsg;
    struct WBArg     *wbargs, *arg;  
    UBYTE            *filename;
    int		activek, inactivek, k;
    LONG 	error = 0L;
    BOOL	active, Force=FALSE;


    FromWb = (argc==0) ? TRUE : FALSE;
    if(FromWb) WBenchMsg = (struct WBStartup *)argv;


    /* Do this BEFORE printing any error messages */
    if(LocaleBase = OpenLibrary("locale.library",0))	/* not required */
    	{
	OpenStrings();
	}

/*
 * Check for usage errors
 */
    if((FromWb)&&(WBenchMsg->sm_NumArgs == 1))	    usage(), bye("",RETURN_OK);

    if((!FromWb)&&((argc<2)||(argv[1][0]=='?')))    usage(), bye(" ",RETURN_OK);

    D(bug(VSTRING));
/*
 * Open Libraries
 */
    if(!(GfxBase = OpenLibrary("graphics.library",0)))
	bye(S(MSG_DI_NOMEM),RETURN_FAIL);

    if(!(IntuitionBase = OpenLibrary("intuition.library",0)))
	bye(S(MSG_DI_NOMEM),RETURN_FAIL);

    if(!(IFFParseBase = OpenLibrary("iffparse.library",0)))
	{
    	if(!(IFFParseBase = OpenLibrary(":libs/iffparse.library",0)))
	    {
	    bye(S(MSG_DI_NOIFFPARSE),RETURN_FAIL);
	    }
	}

/*
 * Other shared allocations for Display
 */

    if(!(pdata = (UWORD *)AllocMem(PDATASZ,MEMF_CHIP|MEMF_CLEAR)))
	bye(S(MSG_DI_NOMEM),RETURN_FAIL);
    pdata[2] = 0x8000;

    mainTask = (struct Task *)FindTask(NULL);
    if((tSigNum = AllocSignal(-1)) == -1)
	bye(S(MSG_DI_NOMEMSIG),RETURN_FAIL);
    tSig = 1 << tSigNum;

    subTask = (struct Task *)CreateTask(subTaskName,0,subTaskRtn,4000);
    if(!subTask)  bye(S(MSG_DI_NOMEM),RETURN_FAIL);

/* 
 * Alloc two IFF handles (so we can load one while displaying another) 
 */
    if(!(iffs[0] = AllocIFF())) bye(S(MSG_DI_NOIFF),RETURN_FAIL);
    if(!(iffs[1] = AllocIFF())) bye(S(MSG_DI_NOIFF),RETURN_FAIL);
    
    if(!(ilbms[0] = (struct ILBMInfo *)
	AllocMem(UICOUNT * sizeof(struct ILBMInfo),MEMF_PUBLIC|MEMF_CLEAR))) 
		bye(S(MSG_DI_NOMEM),RETURN_FAIL);
    else 
	{
	ilbms[1] = ilbms[0] + 1;
	ilbms[2] = ilbms[0] + 2;
	}
/*
 * Set up default ILBMInfo
 */
    ilbms[DEFINFO]->stitle = S(MSG_DI_SCREENTITLE);
    ilbms[DEFINFO]->stype = CUSTOMSCREEN|SCREENBEHIND;
    ilbms[DEFINFO]->windef = &mynw;
    ilbms[DEFINFO]->ParseInfo.propchks = ilbmprops;
    ilbms[DEFINFO]->ParseInfo.collectchks = ilbmcollects;
    ilbms[DEFINFO]->ParseInfo.stopchks = ilbmstops;

/*
 * Get CLI or Workbench options, and filecount
 */
    active = 0;
    if(FromWb)
	{
	filecount = WBenchMsg->sm_NumArgs - 1;
	wbargs = WBenchMsg->sm_ArgList;
	/* The Workbench arg of the Display Tool */
	arg = &wbargs[0];
	startLock  = (BPTR)CurrentDir(arg->wa_Lock);
	/* get Display's ToolTypes */
	getToolOpts(arg);

	/* Do this once now in case project is a file list */
	arg = &wbargs[1];
	/* CD to where this Project file is */
	CurrentDir(arg->wa_Lock);
	/* Get Project's ToolTypes and name */
	getProjectOpts(arg,ilbms[active]);
	}
    else /* from CLI */
	{
	for(k=1; (k<argc) && (stricmp(argv[k],"opt")); k++);
	filecount = k-1;
	if((filecount == 2)&&(!stricmp(argv[1],"from")))
	    {
	    if(filecount = getfilelist(argv[2]))  FileList=TRUE;
	    else bye(S(MSG_DI_NOFILELIST),RETURN_WARN);
	    } 
	/* Get CLI command line opts */
	if((k < argc)&&(!(stricmp(argv[k],"opt"))))  getCliOpts(argc,argv,k);
	}

    if((Timer)&&(dTime < 0)) Timer =FALSE;
    if(filecount == 1)	Loop = FALSE, Mouse = MMouse ? TRUE : FALSE;

    ilbms[DEFINFO]->EHB   = EHB;
    ilbms[DEFINFO]->Autoscroll = Autoscroll;
    ilbms[DEFINFO]->Video = Video;
    ilbms[DEFINFO]->Notransb = Notransb;
    ilbms[DEFINFO]->UserFlags |= IFFPUF_CYCLE;

/*
 * MAIN LOOP - Show the files
 */
    for(k=1; (k<=filecount) && (error != ENDSHOW); k++)
	{
/*
summary of original logic before adding backing-up feature...
     kill inactive if any     
     load current into inactive
     wait for active if any
     toggle active
     show active
*/

	/* If we have an inactive display open, close it */
        if(ilbms[!active]->scr)
	    {
	    enddisplay(ilbms[!active]);
	    }


        /* init individual userinfo to global defaults */
        *ilbms[!active] = *ilbms[DEFINFO];
    	ilbms[!active]->ParseInfo.iff = iffs[!active];

	if(FileList)
	    {
	    filename = fromarray[k];
	    getOptsFromString(fromopts[k],ilbms[!active]);
	    }
	else if(FromWb)
	    {
	    /* Passed file args via Workbench */
	    arg = &wbargs[k];
	    /* CD to where this Project file is */
	    CurrentDir(arg->wa_Lock);
	    /* Get Project's ToolTypes and name */
	    getProjectOpts(arg,ilbms[!active]);
	    filename   = (char *)arg->wa_Name;
	    }
	else /* from CLI */
	    {
	    /* Get filepath/name from command line */
	    filename = argv[k];
	    }


	/* Preload current file as inactive display */
	error = preload(ilbms[!active],filename);

	D(bug("PRELOADED %s #%ld,error = %ld\n\n",filename,k,error));

	if(error)
	    {
	    if((error == NOFILE)&&((k < filecount-1)||(Loop)))
		{
		error = NEXTPIC;
		Force = FALSE;
		}
	    }
	/* If preload ok, wait for any active display to finish */
	else
	    {
	    inactivek = k;
	    /* Force display of preload if we backed up or looped */
	    if(Force)
		{

	D(bug("Setting Force display of preload %ld...\n",k));

		Force = FALSE;
		error = NEXTPIC;
		}
            else
		{
		if(ilbms[active]->scr)
		    {
	D(bug("About to wait on screen $%lx\n ",ilbms[active]->scr));

		    /* Handle messages or timeout of active display */
		    error = handlemsgs(ilbms[active]);

	            D(bug("\nACTIVE back from handlemsgs, error=%ld\n",error));
		    }
		else error = NEXTPIC;
		}
	    
	    if(error == NEXTPIC)
	    	{
	    	/* Toggle index to other set of structures */
	    	active = !active;
	    	activek = inactivek;
	    	/* show active display */
	        D(bug("Bringing activek %ld screen $%lx to front\n",
		    activek,ilbms[active]->scr));
		D(bug("Opts e=%ld n=%ld v=%ld\n",
		    ilbms[active]->EHB,ilbms[active]->Notransb,ilbms[active]->Video));

	    	show(ilbms[active]);


	    	/* special logic for last file */
	    	if(k == filecount)
		    {
		    D(bug("Wait for last screen, k==filecount==%ld\n.. ",k));

		    error = handlemsgs(ilbms[active]);
		    }
	    	}

	    if(error == PREVPIC)
	    	{
		D(bug("\nGot return of PREVPIC, k was %ld... ",k));

	    	if((Loop)&&(activek==1)) k = filecount-1;
	    	else k = (activek >=2) ? activek - 2 : 0;
	    	Force = TRUE;

		D(bug(" k now %ld\n",k));
	    	}
	    }

	if((Loop)&&(error == NEXTPIC)&&(k == filecount))
	    {
	    k=0;
	    Force = TRUE;
	    }
	}

/*
 * Finish up and exit
 */
    /* close all displays */
    if(ilbms[!active]->scr) enddisplay(ilbms[!active]);
    if(ilbms[ active]->scr) enddisplay(ilbms[ active]);

    if((error < 0)||(error == NOFILE)) 	error = RETURN_WARN;
    else if(error < 5)   		error = 0;    /* internal codes */

    bye("",error);
    }


LONG handlemsgs(struct ILBMInfo *ilbm)
{
LONG error = 0L;
ULONG signals, wSig;

    wSig = 1 << ilbm->win->UserPort->mp_SigBit;

    while(! error)
	{
	signals = Wait(SIGBREAKF_CTRL_D|SIGBREAKF_CTRL_C|wSig|tSig);
	if(signals & tSig)  error = NEXTPIC;
	if(signals & SIGBREAKF_CTRL_C)  error = NEXTPIC;
	if(signals & wSig)  error = chkimsgs(ilbm);
	if(signals & SIGBREAKF_CTRL_D)  error = ENDSHOW;
	}
    return(error);
}


LONG preload(struct ILBMInfo *ilbm, UBYTE *filename)
{
LONG error = 0L;

    if(error = showilbm(ilbm, filename))
	{
	unshowilbm(ilbm);
	message("%s: %s\n",filename,IFFerr(error));
	}
    else 
	{
	if(ilbm->ParseInfo.hunt) message("%s: %s\n",filename,S(MSG_DI_HUNTED));
	if(ilbm->Bmhd.nPlanes > 6) message("%s: %ld %s\n",
		filename,ilbm->Bmhd.nPlanes,S(MSG_DI_TOODEEP));
	}
    return(error);
}


void enddisplay(struct ILBMInfo *ilbm)
{
	unshowilbm(ilbm);
}

LONG show(struct ILBMInfo *ilbm)
{
LONG error = 0L, perror;
		
    SetPointer(ilbm->win,pdata,1,16,0,0);
    if(!Back)
	{
	ScreenToFront(ilbm->win->WScreen);
	ActivateWindow(ilbm->win);
	}

    /* Autoprint */

    D(bug("Doing auto screendump if Print>0: Print=%ld\n",Print));

    if(Print)
	{
	if(perror = 
	   screendump(ilbm->win->WScreen,0,0,ilbm->Bmhd.w,ilbm->Bmhd.h,0,0))
		message(S(MSG_DI_PRTTROUBLE_D),perror);
	}
 
    /* Timer */
    if(Timer)  dTimer = dTime, TimerOn = TRUE;
    else TimerOn = FALSE;

    /* Color Cycling */
    /* if(Cycle)  CycleOn = TRUE; */

    D(bug("display Mouse=%ld Loop=%ld Print=%d EHB=%d TimerOn=%d Timer=%d dTime=%ld\n",
				Mouse,Loop,EHB,Print,TimerOn,Timer,dTime));

    return(error);
}


LONG chkimsgs(struct ILBMInfo *ilbm)
   {
   struct Window *win = ilbm->win;
   struct IntuiMessage *msg;
   ULONG class, code;
   SHORT mouseX, mouseY;
   LONG  error = 0L, perror;

   while(msg=(struct IntuiMessage *)GetMsg(win->UserPort))
      {
      class = msg->Class;
      code  = msg->Code;
      mouseX = msg->MouseX;
      mouseY = msg->MouseY;

      ReplyMsg(msg);
      switch(class)
         {
         case MOUSEBUTTONS:
            if ((code == SELECTDOWN)&&
                  (((mouseX < 10)&&(mouseY<10))||(Mouse)))
               {
               error = NEXTPIC;
               }
            else if ((code == SELECTDOWN)&&
                       ((mouseY>10)||(mouseX>10))&&
                          (ilbm->TBState==FALSE))
               {
               ilbm->TBState = TRUE;
               ShowTitle(win->WScreen,TRUE);
               ClearPointer(win);
               }
            else if ((code == SELECTDOWN)&&
                       (mouseY>10)&&(ilbm->TBState==TRUE))
               {
               ilbm->TBState = FALSE;
               ShowTitle(win->WScreen,FALSE);
               SetPointer(win,pdata,1,16,0,0);
               }
	    else if ((code == MENUDOWN)&&(Mouse))
		{
		error = PREVPIC;
		}
            break;
         case VANILLAKEY:
            switch(code)
               {
               case 0x03: case ' ': case '\n':
                  error = NEXTPIC;
                  break;
               case 0x08: case 'b':
                  error = PREVPIC;
                  break;
               case 0x04:
                  error = ENDSHOW;

		  D(bug("Got a CTRL-D from display\n"));

                  break;
               case 0x10:  /* CTRL-P */

		  D(bug("Doing screendump\n"));
    		  if(perror = screendump(ilbm->win->WScreen,0,0,
					  ilbm->Bmhd.w,ilbm->Bmhd.h,0,0))
			message(S(MSG_DI_PRTTROUBLE_D),perror);
                  break;
               default:
                  break;
               }
            break;
         default:
            break;
         }
      }
   return(error);
   }



void usage()
   {
   int k;

   printf("%s\n",Copyright);
   for(k=MSG_DI_USEFIRST; k<=MSG_DI_USELAST; k++) printf("%s\n",S(k));
   if(FromWb)	Delay(500);
   }


void bye(s,rcode)
UBYTE  *s;
LONG  rcode;
   {
   if(*s)
	{
	printf("%s\n",s);
	if(FromWb)
	    {
	    printf(S(MSG_DI_PRESSRET));
	    getchar();
	    }
	}
   cleanup();
   exit(rcode);
   }


void cleanup()
   {
   D(bug("About to Kill subtask\n"));
   if(subTask)
      {
      CycleOn = FALSE;
      PrepareToDie = TRUE;
      while(PrepareToDie)  Delay(10);
      Forbid();
      DeleteTask(subTask);
      Permit();
      }

   D(bug("About to free signal and pointer mem\n"));

   if (tSigNum > -1)  FreeSignal(tSigNum);
   if (pdata)   FreeMem(pdata,PDATASZ);

   D(bug("About to FreeIFF\n"));

   if (ilbms[0]) FreeMem(ilbms[0],UICOUNT * sizeof(struct ILBMInfo));
   if (iffs[0]) FreeIFF(iffs[0]);
   if (iffs[1]) FreeIFF(iffs[1]);

   if (fromtext)	FreeMem(fromtext,ftsz);
   if (fromarray)	FreeMem(fromarray,fasz);

   if (IFFParseBase)	CloseLibrary(IFFParseBase);
   if (IntuitionBase)	CloseLibrary(IntuitionBase);
   if (GfxBase)		CloseLibrary(GfxBase);
   if (LocaleBase)
	{
	CloseStrings();
	CloseLibrary(LocaleBase);
	}
   if (startLock)  	CurrentDir(startLock);
   }


/* global options from CLI */
void getCliOpts(argc,argv,optk)
int  argc;
char **argv;
int optk;
    {
    int k,i;
    UBYTE c;

    for(k=optk+1; k<argc; k++)
      	{
      	c = argv[k][0] | 0x20;

      	for(i=0; argv[k][i]; i++)
      	    {
            c = argv[k][i] | 0x20;
            switch(c)
            	{
		case 't':
			while((argv[k][i])&&(argv[k][i] != '=')) i++;
			i++;
		    	Timer = TRUE;
            		dTime = 60 * atoi(&argv[k][i]);
			while(argv[k][i] > ' ') i++; 
            		break;
    		/* If they ask for MOUSE twice, we will override 1-pic turnoff */
                case 'm':	MMouse=Mouse; Mouse=TRUE;	break;
                case 'l':	Loop = TRUE;			break;
                case 'b':	Back = TRUE;			break;
                case 'p':	Print = TRUE;			break;
                case 'c':	Cycle = TRUE;			break;
                case 'e':	EHB = TRUE;			break;
		case 'n':	Notransb = TRUE;		break;
		case 'a':	Autoscroll = TRUE;		break;
		case 'v':	Video = TRUE;			break;
                default:					break;
                }
	    }
	}
    }


void getToolOpts(wbArg)
struct WBArg  *wbArg;
   {
   struct DiskObject *diskobj;
   char **toolarray;

   if((IconBase = OpenLibrary("icon.library", 0)))
      {
      /* Get ToolTypes from Display.info */
      diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name);
      if(diskobj)
         {
         toolarray = (char **)diskobj->do_ToolTypes;
	 getToolOrListOpts(toolarray);
         FreeDiskObject(diskobj);
         }
      CloseLibrary(IconBase);
      }
   }


/* global options for Tool icon or FileList icon */
void getToolOrListOpts(char **toolarray)
    {
    char *s;
    BOOL m;

    /* If they ask for MOUSE twice, we will override 1-pic turnoff */
    m     = ttTrue(toolarray,"MOUSE");
    if(m)
	{
	MMouse = Mouse;
	Mouse = m;
	}

    Loop  = ttTrue(toolarray,"LOOP");
    Back  = ttTrue(toolarray,"BACK");
    Print = ttTrue(toolarray,"PRINT");
    Cycle = ttTrue(toolarray,"CYCLE");
    EHB   = ttTrue(toolarray,"EHB");
    Autoscroll = ttTrue(toolarray,"AUTOSCROLL");
    Video = ttTrue(toolarray,"VIDEO");
    Notransb = ttTrue(toolarray,"NOTRANSB");

    if(s=(char *)FindToolType(toolarray,"TIMER"))
        {
	Timer =TRUE;
        dTime = 60 * atoi(s);
        }
    }


/* individual options for project icon */
void getProjectOpts(wbArg,ilbm)
struct WBArg  *wbArg;
struct ILBMInfo *ilbm;
    {
    struct DiskObject *diskobj;
    char **toolarray;

    if((IconBase = OpenLibrary("icon.library", 0)))
	{
	/* Get ToolTypes from Display.info */
	diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name);
	if(diskobj)
	    {
	    toolarray = (char **)diskobj->do_ToolTypes;

	    if(ttTrue(toolarray,"FILELIST"))
		{
		if(filecount = getfilelist(wbArg->wa_Name)) FileList = TRUE;
		getToolOrListOpts(toolarray);
		}
	    else
		{	
	    	if(ttTrue(toolarray,"CYCLE")) ilbm->UserFlags |= IFFPUF_CYCLE;
	    	if(ttTrue(toolarray,"EHB"))	  ilbm->EHB = TRUE;
		if(ttTrue(toolarray,"NOTRANSB"))  ilbm->Notransb = TRUE;
		if(ttTrue(toolarray,"AUTOSCROLL"))ilbm->Autoscroll = TRUE;
	    	if(ttTrue(toolarray,"VIDEO"))	  ilbm->Video=TRUE;
		}
	    FreeDiskObject(diskobj);
	    }
	}
    CloseLibrary(IconBase);
    }

/* individual options for file in a FileList */
void getOptsFromString(UBYTE *opts, struct ILBMInfo *ilbm)
    {
    UBYTE c;
    int i;
    
    if(!opts)	return;
    for(i=0; c=opts[i]; i++)
	{
	switch(c)
	    {
	    case 'e':	ilbm->EHB = TRUE;		break;
	    case 'c':	ilbm->UserFlags |= IFFPUF_CYCLE;break;
	    case 'v':   ilbm->Video = TRUE;		break;
	    case 'a':   ilbm->Autoscroll = TRUE;	break;
	    case 'n':   ilbm->Notransb = TRUE;		break;
	    }
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


void subTaskRtn(void)
    {
    geta4();

    while(!PrepareToDie)
	{
	WaitTOF();
	doCycle();

	if(TimerOn)
            {
            if(dTimer == 0) Signal(mainTask,tSig), TimerOn=FALSE;
	    dTimer--;
	    }
	}
    PrepareToDie = FALSE;
    Wait(0L);  /* Wait to die */
    }



void doCycle()
    {
#ifdef CYCLE IN
    int    k, i, j;
    UBYTE  low, high;
    USHORT cyTmp;
#endif
    BOOL   Cycled;

    Cycled = FALSE;
#ifdef CYCLEIN
    for(k=0; k<cyCnt; k++)
	{
	if(cyRates[k])  /* cyRate 0 = inactive */
	    {
	    cyClocks[k] += cyRates[k];
	    if(cyClocks[k] >= CYCLETIME)
		{
		Cycled = TRUE;
		cyClocks[k] -= CYCLETIME;
		low = cyCrngs[k].low;
		high= cyCrngs[k].high;
		if(cyCrngs[k].active & REVERSE)  /* Reverse cycle */
		    {
		    cyTmp = cyMap[low];
		    for(i=low,j=low+1; i < high; i++,j++)
			{
			cyMap[i] = cyMap[j];
			}
		    cyMap[high] = cyTmp;
		    }
		else     /* Forward cycle */
		    {
		    cyTmp = cyMap[high];
		    for(i=high,j=high-1; i > low; i--,j--)
			{
			cyMap[i] = cyMap[j];
			}
		    cyMap[low] = cyTmp;
		    }
		}
	    }
	}
    if(Cycled)
	{
	LoadRGB4(cyVport,cyMap,cyRegs);
	}
#endif
    }



/* passed name of FROM file, sets up fromtext and fromarray. 
 * filecount returned
 */
ULONG getfilelist(UBYTE *fromfile)
{
LONG file,i;
ULONG lcnt=0, fcnt=0;
UBYTE c;
BOOL HaveText=FALSE, InQuotes=FALSE;

    if(file = Open(fromfile,MODE_OLDFILE))
	{
	Seek(file,0,OFFSET_END);
	ftsz = Seek(file,0,OFFSET_BEGINNING);

	if(fromtext = AllocMem(ftsz,MEMF_PUBLIC))
	    {
	    Read(file,fromtext,ftsz);
	    }
	Close(file);
	if(!fromtext) return(0L);

	for(i=0; i<ftsz; i++) if(fromtext[i]=='\n') lcnt++;

	fasz = (lcnt+1) << 4;
	if(!(fromarray = (UBYTE **)AllocMem(fasz, MEMF_PUBLIC|MEMF_CLEAR)))
	    {
	    FreeMem(fromtext,ftsz);
	    return(0L);
	    }

	fromopts = fromarray + (lcnt + 1);

	for(fcnt=0,i=0; i<ftsz; i++)
	    {
	    c = fromtext[i];
	    if((!HaveText)&&(c!='\n')&&(c!=' '))
		{
		if(c == '"') InQuotes = TRUE;
		else
		    {
		    HaveText=TRUE;
		    fcnt++;   /* we start in element 1 to match main loop */
		    fromarray[fcnt] = &fromtext[i];
		    }
		}
	    else if(c=='"')
		{
		fromtext[i] = '\0';
		InQuotes = FALSE;
		}
	    else if((HaveText)&&((c=='\n')||((!InQuotes)&&(c==' '))))
		{
		fromtext[i] = '\0';   /* terminate filename */
		HaveText = FALSE;
		if(c != '\n')  /* if not end of line */
		    {
		    fromopts[fcnt] = &fromtext[i+1];
		    while((i < ftsz)&&(fromtext[i] != '\n')) i++;
		    if(i<ftsz) fromtext[i] = '\0';
		    else fromtext[i-1] = '\0';
		    }
		}
	    }
	}
    return(fcnt);
}

/*------------------------------------------------------------------------*/




