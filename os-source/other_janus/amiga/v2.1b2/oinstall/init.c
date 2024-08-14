/*
   Initialization and ShutDown Code.
*/

#include "exec/types.h"
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <exec/memory.h>
#include "extern.h"

#include "protos.h"

void exit(long);

#define PATHLEN   128
#define STITLE   "                         Installation Utility 1.2"
#define VERSION 33
#define REVISION 3

extern struct Gadget UpGadget, OkGadget, CnGadget;

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *IconBase = NULL;

static struct Process *MyProc;
static APTR OldWindowPtr;

static struct NewScreen ns = {
   0, 0,         /* LeftEdge, Height */
   SWIDTH, SHEIGHT,/* Width, Height */
   2,            /* Depth */
   0, 1,         /* DetailPen, BlockPen */
   HIRES,         /* ViewModes */
   CUSTOMSCREEN,   /* Type */
   NULL,         /* Font */
   STITLE,         /* Default Title */
   NULL,         /* Gadgets */
   NULL         /* CustomBitMap */
};

static struct NewWindow nw = {
   0, WTOPEDGE,      /* left edge, top edge */
   WWIDTH, WHEIGHT,   /* width, height */
   0, 1,            /* DetailPen, BlockPen */
   (MOUSEBUTTONS|GADGETUP|GADGETDOWN|CLOSEWINDOW),   /* IDCMP Flags */
   (SMART_REFRESH|ACTIVATE|NOCAREREFRESH|BACKDROP|WINDOWCLOSE), /* Flags */
   &UpGadget,         /* First Gadget */
   NULL,            /* CheckMark */
   "  ",            /* Title */
   NULL,            /* Screen */
   NULL,            /* BitMap */
   0, 0,            /* MinWidth, MinHeight */
   0, 0,            /* MaxWidth, MaxHeight */
   CUSTOMSCREEN      /* Type */
};

static struct IntuiText BodyIText = {
   AUTOFRONTPEN,   /* FrontPen */
   AUTOBACKPEN,   /* BackPen */
   AUTODRAWMODE,   /* DrawMode */   
   8,            /* LeftEdge */
   AUTOTOPEDGE,   /* TopEdge */
   NULL,         /* ITextFont */
   "I don't know what to install!", /* IText */
   NULL         /* NextText */
};

static struct IntuiText NegIText = {
   AUTOFRONTPEN,   /* FrontPen */
   AUTOBACKPEN,   /* BackPen */
   AUTODRAWMODE,   /* DrawMode */   
   AUTOLEFTEDGE,   /* LeftEdge */
   AUTOTOPEDGE,   /* TopEdge */
   NULL,         /* ITextFont */
   "OK",         /* IText */
   NULL         /* NextText */
};

VOID BuildAbsolutePath(UBYTE *name,UBYTE *buf,ULONG len)
{
   struct FileLock *lock, *lock2;
   struct FileInfoBlock *fib;
   UBYTE *tmpbuf;
   UBYTE *pname = name;
   UBYTE *pbuf = buf;
   D( kprintf("BAP: pname='%s'\n", pname); )
   *pbuf = '\0';
   if ((fib = AllocMem(sizeof(struct FileInfoBlock), MEMF_PUBLIC)) != NULL) {
      if ((tmpbuf = AllocMem(len, MEMF_PUBLIC)) != NULL) {
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
                  }
                  else {
                     strcat(tmpbuf, ":");
                  }
               }
               strcat(tmpbuf, pbuf);
               strcpy(pbuf, tmpbuf);
            }
            else {
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
         FreeMem(tmpbuf, len);
      }
#if DEBUG
      else {
         kprintf("BAP: Couldn't get mem for tmpbuf\n");
      }
#endif
      FreeMem(fib, sizeof(struct FileInfoBlock));
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

LONG StartUp(int argc,UBYTE *argv[],UBYTE *installbuf[],UBYTE *defbuf,UBYTE *lastbuf,UBYTE *basebuf)
{
   struct WBArg *WArgv;
   struct WBStartup *WBStart;
   struct DiskObject *WBDiskObject;
   long WBArgs;
   LONG err=0;
   LONG nb=0;
   LONG i;
   UBYTE *string, **ToolArray, *pstring;
   UBYTE buf[PATHLEN];

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

      if ((WBDiskObject=GetDiskObject(WArgv[1].wa_Name)) != NULL) {
          ToolArray = WBDiskObject->do_ToolTypes;
          if ((string = FindToolType(ToolArray, "DRAWER")) != NULL) {
              strcpy(buf,string);
         for(pstring= &buf[0]; *pstring != 0; pstring++) {
            if(*pstring == '|') *pstring=0;
         }
         for(string= &buf[0]; string < pstring; 
                  string += (strlen(string)+1)) { 
             BuildAbsolutePath(string, installbuf[nb++], PATHLEN);
                  }
          }
      else {
          err++;
          D( kprintf("SU: string was NULL\n"); )
      }
      if ((string = FindToolType(ToolArray, "LAST")) != NULL)
          BuildAbsolutePath(string, lastbuf, PATHLEN);
      if ((string = FindToolType(ToolArray, "DEF")) != NULL)
          BuildAbsolutePath(string, defbuf, PATHLEN);
      }
      D( else kprintf("SU: Couldn't GetDiskObject\n"); )
   }
      D( else kprintf("SU: WBArgs=%ld\n", WBArgs); )
       }
   else { /* CLI */
#if DEBUG
      kprintf("SU: CLI, argc=%ld\n", argc);
      for (i=0; i<argc; i++) {
         kprintf("argv[%ld]='%s'\n", i, argv[i]);
      }
#endif
      if (argc >= 2) {
          BuildAbsolutePath(argv[0], basebuf, PATHLEN);
          basecopy(basebuf,basebuf); /* get disk name */
          for(i=1; i<argc; i++) {
         if(!(stricmpn("def=",argv[i],4))) {
             BuildAbsolutePath(&(argv[i][4]),defbuf,PATHLEN);
         }
         else if(!(stricmpn("last=",argv[i],5))) {
             BuildAbsolutePath(&(argv[i][5]),lastbuf,PATHLEN);
         }
         else BuildAbsolutePath(argv[i],installbuf[nb++],PATHLEN);
          }
      }
      else err++;
   }
#if DEBUG
   kprintf("base=%s\n",basebuf);
   for(i=0; i<nb; i++) kprintf("install[%ld]=%s\n",i,installbuf[i]);
   kprintf("def=%s\n",defbuf);
   kprintf("last=%s\n",lastbuf);
#endif
   if (err) {
       if(!argc)AutoRequest(NULL, &BodyIText, NULL, &NegIText, 0, 0,
         strlen(BodyIText.IText) * 8 + 40, 56);
   else printf("Usage: %s directory\n", argv[0]); /* CLI error */
   }
   return(err); /* return true or false */
}

LONG Init(int argc,UBYTE *argv[],UBYTE *installbuf[],UBYTE *defbuf,UBYTE *lastbuf,UBYTE *basebuf)
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

   if (StartUp(argc, argv, installbuf, defbuf, lastbuf,basebuf)) {
      D( kprintf("I: Couldn't StartUp\n"); )
      return(0);
   }

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
   Status(w, 0, 0, 0);
   rp = w->RPort;
   SetAPen(rp, 1);
   Move(rp, CHARXOFFSET, TOPOFFSET);
   Draw(rp, WWIDTH-1, TOPOFFSET);         /* top line */
   
   D( kprintf("I: exit\n"); )
   return(1);
}

VOID Bye(long err)
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

   Delay(300);
   D( kprintf("B: exit\n"); )
   exit(err);
}
