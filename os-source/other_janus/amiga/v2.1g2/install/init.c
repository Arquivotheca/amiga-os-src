/*
	Initialization and ShutDown Code.
*/

#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <proto/all.h>

#include "install.h"
#include "intui.h"
#include "display.h"
#include "init.h"

#define D(x) ;
#undef DEBUG

#define PATHLEN	128
#define STITLE	"                         Installation Utility 2.1"
#define VERSION 36
#define REVISION 1

extern struct Gadget UpGadget, OkGadget, CnGadget;

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *IconBase = NULL;

static struct Process *MyProc;
static APTR OldWindowPtr;

static struct NewScreen ns = {
	0, 0,			/* LeftEdge, Height */
	SWIDTH, SHEIGHT,	/* Width, Height */
	2,			/* Depth */
	0, 1,			/* DetailPen, BlockPen */
	HIRES,			/* ViewModes */
	CUSTOMSCREEN,		/* Type */
	NULL,			/* Font */
	STITLE,			/* Default Title */
	NULL,			/* Gadgets */
	NULL			/* CustomBitMap */
};

static struct NewWindow nw = {
	0, WTOPEDGE,		/* left edge, top edge */
	WWIDTH, WHEIGHT,	/* width, height */
	0, 1,			/* DetailPen, BlockPen */
	(MOUSEBUTTONS|GADGETUP|GADGETDOWN|CLOSEWINDOW),	/* IDCMP Flags */
	(SMART_REFRESH|ACTIVATE|NOCAREREFRESH|BACKDROP|WINDOWCLOSE),	/* Flags */
	&UpGadget,		/* First Gadget */
	NULL,			/* CheckMark */
	"  ",			/* Title */
	NULL,			/* Screen */
	NULL,			/* BitMap */
	0, 0,			/* MinWidth, MinHeight */
	0, 0,			/* MaxWidth, MaxHeight */
	CUSTOMSCREEN		/* Type */
};

static struct IntuiText BodyIText = {
	AUTOFRONTPEN,		/* FrontPen */
	AUTOBACKPEN,		/* BackPen */
	AUTODRAWMODE,		/* DrawMode */	
	8,			/* LeftEdge */
	AUTOTOPEDGE,		/* TopEdge */
	NULL,			/* ITextFont */
	"I don't know what to install!", /* IText */
	NULL			/* NextText */
};

static struct IntuiText NegIText = {
	AUTOFRONTPEN,		/* FrontPen */
	AUTOBACKPEN,		/* BackPen */
	AUTODRAWMODE,		/* DrawMode */	
	AUTOLEFTEDGE,		/* LeftEdge */
	AUTOTOPEDGE,		/* TopEdge */
	NULL,			/* ITextFont */
	"OK",			/* IText */
	NULL			/* NextText */
};

VOID basecopy(UBYTE *dest,UBYTE *source)
{
   while ((*source != ':')&&( *dest++ = *source++ ));
   *dest=0;
}

LONG stricmpn(UBYTE *s, UBYTE *t, LONG n)
{
   char c1, c2;
   LONG i=0;

   do {
      c1 = *s++;
      if (c1 >= 'a' && c1 <= 'z') {
         c1 = toupper(c1);
      }
      c2 = *t++;
      if (c2 >= 'a' && c2 <= 'z') {
         c2 = toupper(c2);
      }
      if (c1 == '\0' || c2 == '\0') {
         break;
      }
   } while ((c1 == c2) && (i++ < n));

   if(i==n)return(0);

   if (c1 == '\0') {
      if (c2 == '\0')return(0);
      return(1);
   }
   return(-1);
}

VOID BuildAbsolutePath(UBYTE *name,UBYTE *buf, LONG len)
{
struct FileLock *lock, *lock2;
struct FileInfoBlock *fib;
UBYTE *tmpbuf;
UBYTE *pname = name;
UBYTE *pbuf = buf;

	D( kprintf("BAP: pname='%s'\n", pname); )
	*pbuf = '\0';
	if ((fib = my_malloc(sizeof(struct FileInfoBlock))) != NULL) {
		if ((tmpbuf = my_malloc(len)) != NULL) {
			if ((lock2 = (struct FileLock *)Lock(pname, ACCESS_READ)) != NULL) {
				do {
					lock = lock2; /* offspring becomes parent */
					lock2 = (struct FileLock *)ParentDir((BPTR)lock); /* get lock on parent */
					if (Examine((BPTR)lock, fib)) {
						strcpy(tmpbuf, fib->fib_FileName);
						D( kprintf("DirEntryType=%ld\n", fib->fib_DirEntryType); )
						if (fib->fib_DirEntryType > 0) {
							if (lock2 != NULL) {
								strcat(tmpbuf, "/");
							} else {
								strcat(tmpbuf, ":");
							}
						}
						strcat(tmpbuf, pbuf);
						strcpy(pbuf, tmpbuf);
					} else {
						D( kprintf("Couldn't examine lock\n"); )
						UnLock((BPTR)lock); /* unlock offspring */
						UnLock((BPTR)lock2); /* unlock parent */
						break;
					}
					UnLock((BPTR)lock); /* unlock offspring */
				} while (lock2 != NULL);
			}
#if DEBUG
			else {
				kprintf("BAP: Couldn't get lock on '%s'\n", pname);
			}
#endif
			my_free(tmpbuf);
		}
#if DEBUG
		else {
			kprintf("BAP: Couldn't get mem for tmpbuf\n");
		}
#endif
		my_free(fib);
	}
#if DEBUG
	else {
		kprintf("BAP: Couldn't get mem for fib\n");
	}
#endif
	if (*pbuf != '\0' && pbuf[strlen(pbuf) - 1] == '/') {
		pbuf[strlen(pbuf) - 1] = '\0'; /* overwrite last '/' */
	}

	D( kprintf("BAP: pbuf='%s'\n\n", pbuf); )
}

LONG StartUp(int argc,UBYTE *argv[],UBYTE *installbuf,UBYTE *defbuf,UBYTE *basebuf,UBYTE *destbuf)
{
struct WBArg *WArgv;
struct WBStartup *WBStart;
struct DiskObject *WBDiskObject;
long WBArgs;
LONG err=0;
LONG i;
UBYTE *string, **ToolArray;

	/* D( UBYTE *ptr; ) */

	if (!argc) { /* WorkBench */
		 WBStart = (struct WBStartup *)argv;
		 WArgv = WBStart->sm_ArgList;
		 WBArgs = WBStart->sm_NumArgs;
#if DEBUG
		for (i=0; i<WBArgs; i++) {
			kprintf("SU: WB: WArgv[%ld].wa_Name='%s'\n",
				i, WArgv[i].wa_Name);
		}
#endif
		if (WBArgs == 2) {
			string=WArgv[1].wa_Name;
			BuildAbsolutePath(string,defbuf,PATHLEN);
			basecopy(basebuf,defbuf); /* put disk name into basebuf */
			strcat(basebuf, ":");

			if ((WBDiskObject=GetDiskObject(WArgv[1].wa_Name)) != NULL) {
				ToolArray = WBDiskObject->do_ToolTypes;
				if ((string = FindToolType(ToolArray, "SOURCE")) != NULL) {
					BuildAbsolutePath(string, installbuf, PATHLEN);
				} else if ((string = FindToolType(ToolArray, "DEFDEL")) != NULL) {
					BuildAbsolutePath(string, defbuf, PATHLEN);
				} else if ((string = FindToolType(ToolArray, "DESTINATION")) != NULL) {
					strcpy(destbuf, string);
				} else {
					err++;
				}
			}
			D( else kprintf("SU: Couldn't GetDiskObject\n"); )
		}
		D( else kprintf("SU: WBArgs=%ld\n", WBArgs); )
	} else { /* CLI */
#if DEBUG
		kprintf("SU: CLI, argc=%ld\n", argc);
		for (i=0; i<argc; i++) {
			kprintf("argv[%ld]='%s'\n", i, argv[i]);
		}
#endif
		if (argc >= 2) {
			BuildAbsolutePath(argv[0], basebuf, PATHLEN);
			basecopy(basebuf,basebuf); /* get disk name */
			strcat(basebuf, ":");
			for(i=1; i<argc; i++) {
				if(!(stricmpn("defdel=",argv[i],7))) {
					BuildAbsolutePath(&(argv[i][7]),defbuf,PATHLEN);
				} else if(!(stricmpn("source=",argv[i],7))) {
					BuildAbsolutePath(&(argv[i][7]),installbuf,PATHLEN);
				} else if (!(stricmpn("destination=", argv[i], 12))) {
					char *p1, *p2;
					p1 = &(argv[i][12]);
					p2 = destbuf;
					while (*p1 && *p1 != ' ' && *p1 != '\t')
						*p2++ = *p1++;
					*p2 = 0;
				} else {
					err++;
				}
			}
		} else {
			err++;
		}
	}
#if DEBUG
	kprintf("base=%s\n",basebuf);
	for(i=0; i<nb; i++)
		kprintf("install[%ld]=%s\n",i,installbuf[i]);
	kprintf("def=%s\n",defbuf);
	kprintf("last=%s\n",lastbuf);
#endif

	if ((strlen(defbuf) == 0) || (strlen(installbuf) == 0) || (strlen(destbuf) == 0) || (strchr(destbuf, ':') == 0))
		err++;

	if (err) {
		if(!argc)
			AutoRequest(NULL, &BodyIText, NULL, &NegIText, 0, 0,
				    strlen(BodyIText.IText) * 8 + 40, 56);
		else
			printf("Usage: %s defdel=fname source=fname destination=volname\n", argv[0]); /* CLI error */
	}

	return(err); /* return true or false */
}

LONG Init(int argc, char **argv, UBYTE *installbuf, UBYTE *defbuf, UBYTE *basebuf, UBYTE *destbuf)
{
	D( kprintf("I: enter\n"); )
	if (!(GfxBase = OpenLibrary("graphics.library", 0))) {
		D( kprintf("I: Couldn't open Gfx Library\n"); )
		return(0);
	}

	if (!(IntuitionBase = OpenLibrary("intuition.library", 0))) {
		D( kprintf("I: Couldn't open Intuition Library\n"); )
		return(0);
	}

	if (!(IconBase = OpenLibrary("icon.library", 0))) {
		D( kprintf("I: Couldn't open Icon Library\n"); )
		return(0);
	}

	if (StartUp(argc, argv, installbuf, defbuf, basebuf, destbuf)) {
		D( kprintf("I: Couldn't StartUp\n"); )
		return(0);
	}

	ns.Font = &topaz8;
	if ((s = OpenScreen(&ns)) == NULL) {
		D( kprintf("I: Couldn't open screen\n"); )
		return(0);
	}

	nw.Screen = s;
	if ((w = OpenWindow(&nw)) == NULL) {
		D( kprintf("I: Couldn't open window\n"); )
		return(0);
	}

	MyProc = (struct Process *)FindTask(NULL);
	OldWindowPtr = MyProc->pr_WindowPtr;
	MyProc->pr_WindowPtr = (APTR)w;
	AddGadget(w, &OkGadget, -1);
	AddGadget(w, &CnGadget, -1);
	RefreshGadgets(&OkGadget, w, NULL);
/*	Status(w, 0, 0, 0);
*/
	rp = w->RPort;
	SetAPen(rp, 1);
	Move(rp, CHARXOFFSET, TOPOFFSET);
	Draw(rp, WWIDTH-1, TOPOFFSET);			/* top line */
	
	D( kprintf("I: exit\n"); )
	return(1);
}

VOID Bye()
{
	D( kprintf("B: enter\n"); )
	if (w) {
		MyProc->pr_WindowPtr = OldWindowPtr;
		CloseWindow(w);
	}
	if (s)CloseScreen(s);
		
	if (IconBase)CloseLibrary(IconBase);

	if (IntuitionBase)CloseLibrary(IntuitionBase);

	if (GfxBase)CloseLibrary(GfxBase);

/*	Delay(300);
*/
	D( kprintf("B: exit\n"); )
/*	exit(err);
*/
}
