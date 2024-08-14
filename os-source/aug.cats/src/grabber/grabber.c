/* grabber
 * Saves front screen as an ILBM
 * Requires linkage with several iffparse modules - See Makefile
 */ 
#define INTUI_V36_NAMES_ONLY

#include "iffp/ilbmapp.h"

#include <intuition/intuitionbase.h>
#include <workbench/workbench.h>
#include <graphics/gfxbase.h>

#include <clib/icon_protos.h>


#ifdef __SASC
void __chkabort(void) {}          /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}            /* Disable LATTICE CTRL-C checking */
#endif
#endif


#include "grabber_rev.h"
UBYTE vers[] = VERSTAG;
UBYTE Copyright[] = VERS " - Saves screen as ILBM - Freely Redistributable";

char *usage =
 "Usage: grabber filename\n" 
 "Opts: QUIET NOICONS NOMONITOR\n"
 "Saves front screen (or ViewPort) from serial terminal\n";

int mygets(char *s);
void bye(UBYTE *s,int e);
void cleanup(void);

struct Library	*IntuitionBase = NULL;
struct Library	*GfxBase = NULL;
struct Library	*IconBase = NULL;
struct Library	*IFFParseBase = NULL;

struct ILBMInfo ilbm = {0};

BOOL fromWB, Quiet, NoDelay, NoIcon, Sequence, FrontVP, NoMonitor;



#define INBUFSZ 128
char nbuf[INBUFSZ];

/* Data for project icon for saved ILBM
 *
 */
UWORD ILBMI1Data[] =
{
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0001,0x0000,0x0000,0x0000,0x0003,
    0x0FFF,0xFFFF,0xFFFF,0xFFF3,0x0FFF,0x0000,0x0000,0xFFF3,
    0x0FFC,0x0000,0x0000,0x3FF3,0x0FE0,0x0E80,0xF800,0x07F3,
    0x0F80,0x1C01,0x8C00,0x01F3,0x0F00,0x0001,0x8C00,0x00F3,
    0x0600,0x0000,0x0600,0x0063,0x0600,0x0003,0xBC00,0x0063,
    0x0600,0x0001,0xFC00,0x0063,0x0600,0x0000,0xFC00,0x0063,
    0x0600,0x1FC1,0xFE40,0x0063,0x0600,0x1DC1,0xFE20,0x0063,
    0x0600,0x1CE3,0xFF12,0x0063,0x0F00,0x1CE0,0x004F,0xC0F3,
    0x0F80,0x1CE0,0x002F,0x01F3,0x0FE0,0x0E78,0x423D,0x07F3,
    0x0FFC,0x0000,0x0000,0x3FF3,0x0FFF,0x0000,0x0000,0xFFF3,
    0x0FFF,0xFFFF,0xFFFF,0xFFF3,0x0000,0x0000,0x0000,0x0003,
    0x7FFF,0xFFFF,0xFFFF,0xFFFF,
/* Plane 1 */
    0xFFFF,0xFFFF,0xFFFF,0xFFFE,0xD555,0x5555,0x5555,0x5554,
    0xD000,0x0000,0x0000,0x0004,0xD3FC,0xFFFF,0xFFFF,0x3FC4,
    0xD3C0,0x0000,0x0000,0x03C4,0xD300,0x0100,0xF800,0x00C4,
    0xD300,0x0381,0xFC00,0x00C4,0xD080,0x0701,0xFC00,0x0104,
    0xD180,0xF883,0xFE00,0x0194,0xD181,0xDF80,0x4700,0x0194,
    0xD181,0xDF82,0x0180,0x0194,0xD180,0x6F82,0x00C0,0x0194,
    0xD180,0x0002,0x0020,0x0194,0xD180,0x0000,0x0000,0x0194,
    0xD180,0x0000,0x0002,0x0194,0xD080,0x0000,0xCC46,0xC104,
    0xD300,0x0000,0xCC2F,0x00C4,0xD300,0x0E78,0x883D,0x00C4,
    0xD3C0,0x0000,0x0000,0x03C4,0xD3FC,0xFFFF,0xFFFF,0x3FC4,
    0xD000,0x0000,0x0000,0x0004,0xD555,0x5555,0x5555,0x5554,
    0x8000,0x0000,0x0000,0x0000,
};

struct Image ILBMI1 =
{
    0, 0,			/* Upper left corner */
    64, 23, 2,			/* Width, Height, Depth */
    ILBMI1Data,			/* Image data */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};

UBYTE *ILBMTools[] =
{
    "FILETYPE=ILBM",
    NULL
};

struct DiskObject ILBMobject =
{
    WB_DISKMAGIC,		/* Magic Number */
    WB_DISKVERSION,		/* Version */
    {				/* Embedded Gadget Structure */
	NULL,			/* Next Gadget Pointer */
	0, 0, 64, 24,		/* Left,Top,Width,Height */
	GFLG_GADGIMAGE | GADGBACKFILL,	        /* Flags */
	GACT_RELVERIFY | GACT_IMMEDIATE,	/* Activation Flags */
	GTYP_BOOLGADGET,		        /* Gadget Type */
	(APTR)&ILBMI1,	/* Render Image */
	NULL,			/* Select Image */
	NULL,			/* Gadget Text */
	NULL,			/* Mutual Exclude */
	NULL,			/* Special Info */
	0,			/* Gadget ID */
	NULL,			/* User Data */
    },
    WBPROJECT,			/* Icon Type */
    "Display",			/* Default Tool */
    ILBMTools,			/* Tool Type Array */
    NO_ICON_POSITION,		/* Current X */
    NO_ICON_POSITION,		/* Current Y */
    NULL,			/* Drawer Structure */
    NULL,			/* Tool Window */
    0				/* Stack Size */
};


/*
*	extern MainTask, WedgedTask, ULONG fcount, and BOOLS DoView, Quiet
*	set MainTask
*	call WedgeLoadView
*	wait for SAVESIG | SIGBREAKF_CTRL_C
*		save screen or view (based on DoView) if SAVESIG, send DONESIG, loop back
*		set Quiet, Delay, and exit if CTRL_C
*
*/

extern struct Task far *MainTask, *WedgedTask;
extern ULONG far fcount;
extern BOOL far DoView, Off;

extern ULONG WedgeLoadView(void);
extern ULONG UnWedgeLoadView(void);

#define SAVESIG SIGBREAKF_CTRL_F
#define DONESIG SIGBREAKF_CTRL_E

UBYTE nextname[80];

void main(int argc, char **argv)
   {
   struct View     *frontview;
   struct ViewPort *frontvp;
   struct BitMap   *frontbitmap;
   UWORD	   *frontcolortable;
   struct Screen   *frontScreen;
   ULONG signals;
   LONG		error = 0L /*, seqlock */ ;
   char         *filename, *basename;
   int l, k;

   fromWB = (argc==0) ? TRUE : FALSE;


   if(!(IntuitionBase = OpenLibrary("intuition.library", 0)))
      bye("Can't open intuition library.\n",RETURN_WARN);
      
   if(!(GfxBase = OpenLibrary("graphics.library",0)))
      bye("Can't open graphics library.\n",RETURN_WARN);

   if(!(IFFParseBase = OpenLibrary("iffparse.library",0)))
      bye("Can't open iffparse library.\n",RETURN_WARN);

   if(!(IconBase = OpenLibrary("icon.library",0)))
      bye("Can't open icon library.\n",RETURN_WARN);

   if(!(ilbm.ParseInfo.iff = AllocIFF()))
      bye(IFFerr(IFFERR_NOMEM),RETURN_WARN);

   MainTask = FindTask(NULL);

   if(!(WedgeLoadView()))
      bye("Can't wedge loadviw",RETURN_FAIL);

   if(argc>1)                 /* Passed filename via command line  */
      {
      if(argv[1][0]=='?') 
	    {
	    printf("%s\n%s\n",Copyright,usage);
	    bye("\n",RETURN_OK);
	    }
      else filename = argv[1];

      NoDelay = NoIcon = Quiet = Sequence = FrontVP = FALSE;
      for(k=2; k < (argc); k++)
	{
	if(!(stricmp(argv[k],"NODELAY")))	 NoDelay  = TRUE;
	else if(!(stricmp(argv[k],"NOICONS")))	 NoIcon   = TRUE;
	else if(!(stricmp(argv[k],"NOICON")))	 NoIcon   = TRUE;
	else if(!(stricmp(argv[k],"NOMONITOR"))) NoMonitor= TRUE;
	else if(!(stricmp(argv[k],"QUIET")))	 Quiet    = TRUE;
	else if(!(stricmp(argv[k],"SEQUENCE")))	 Sequence = TRUE;
	else if(!(stricmp(argv[k],"VIEWPORT")))  FrontVP = TRUE;
	}
      if(NoMonitor)	ilbm.IFFPFlags |= IFFPF_NOMONITOR;

/*
      if(Sequence)
	{
	for(k=1; k<9999; k++)
	    {
	    sprintf(nbuf,"%s%04ld",filename,k);
	    if(seqlock = Lock(nbuf,ACCESS_READ))	UnLock(seqlock);
	    else break;
	    }
	filename = nbuf;
	}
*/

      }
   else
      {
      printf("%s\n%s\n",Copyright,usage);
      printf("Enter filename for save: ");
      l = mygets(&nbuf[0]);

      if(l==0)                /* No filename - Exit */
         {
         bye("\nScreen not saved, filename required\n",RETURN_FAIL);
         }
      else
         {
         filename = &nbuf[0];
         }
      }

/*     
   if(!NoDelay)	Delay(500);
*/

   basename = filename;
   filename = nextname;

loop:
   signals = Wait(SIGBREAKF_CTRL_C | SAVESIG);
   if(signals & SIGBREAKF_CTRL_C)	bye("",RETURN_OK);

   sprintf(nextname,"%s_%04ld",basename,fcount);

   FrontVP = DoView;

   Forbid();
   frontScreen  = ((struct IntuitionBase *)IntuitionBase)->FirstScreen;
   Permit();


   if(FrontVP)
	{
	frontview = ((struct GfxBase *)GfxBase)->ActiView;
	frontvp = frontview->ViewPort;
	while((frontvp->Next)&&(frontvp->Modes & VP_HIDE))
		frontvp = frontvp->Next;
	frontbitmap = frontvp->RasInfo->BitMap;
	frontcolortable = frontvp->ColorMap->ColorTable;
	if(error = saveilbm(&ilbm,
                frontbitmap, (ULONG)frontvp->Modes,
                frontvp->DWidth, frontbitmap->Rows,
                frontvp->DWidth, frontbitmap->Rows,
                frontcolortable, frontvp->ColorMap->Count, 4,
                0, 0,
                NULL,NULL,
                filename))
	printf("%s\n",IFFerr(error));
	}
   else if(error = screensave(&ilbm, frontScreen,
				NULL, NULL,
				filename))
	{
	printf("%s\n",IFFerr(error));
	}

   if(! error)
	{
      	if(!Quiet) printf("%s saved as %s... ",
			FrontVP ? "Viewport" : "Screen",filename);
	if((!NoIcon)&&(filename[0]!='-')&&(filename[1]!='c')) /* not clipboard */
	    {
      	    if(!(PutDiskObject(filename,&ILBMobject)))
            	{
            	bye("Error saving icon\n",RETURN_WARN);
            	}
   	    if(!Quiet) printf("Icon saved\n");
	    }
	else if(!Quiet) printf("\n");
      	}

    Signal((struct Task *)WedgedTask,DONESIG);
    goto loop;
    }


void bye(UBYTE *s,int e)
   {
   if(s&&(*s)) printf("%s\n",s);
   if ((fromWB)&&(*s))    /* Wait so user can read messages */
      {
      printf("\nPRESS RETURN TO EXIT\n");
      mygets(&nbuf[0]);
      }
   cleanup();
   exit(e);
   }

void cleanup()
   {
   if(ilbm.ParseInfo.iff)	FreeIFF(ilbm.ParseInfo.iff);

   if(GfxBase)		CloseLibrary(GfxBase);
   if(IntuitionBase)	CloseLibrary(IntuitionBase);
   if(IconBase)		CloseLibrary(IconBase);
   if(IFFParseBase)	CloseLibrary(IFFParseBase);

   Off = TRUE;
   UnWedgeLoadView();
   Delay(15);
   }


int mygets(char *s)
   {
   int l = 0, max = INBUFSZ - 1;

   while (((*s = getchar()) !='\n' )&&(l < max)) s++, l++;
   *s = NULL;
   return(l);
   }

