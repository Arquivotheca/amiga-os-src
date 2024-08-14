
;/* graphicdump.c - Execute me to compile me with Lattice 5.04
LC -b0 -cfistq -v -y -j73 graphicdump.c
Blink FROM RXstartup.obj,graphicdump.o TO graphicdump LIBRARY LIB:Amiga.lib,LIB:LC.lib
quit
*/

/*
 * GraphicDump.c  - Modified by Darren Greenwald CBM 9-6-90
 *
 *                  Provide support for new graphics modes -
 *                  primarily to accomidate the new aspect
 *                  ratios possible under V36 with the new
 *                  chip set.
 *
 *                  Conditional compilation makes it possible
 *                  to easily back out the V36 enhancements
 *                  in the executable file, thereby reducing
 *                  code size somewhat if needed.
 *
 *                  Still, the enhancements will allow execution
 *                  of this program under < V36.
 *
 *                  What we do is use modeID as returned by
 *                  GetVPModeID() in graphics.library V36
 *                  which is compatable with ViewPort->modes.
 *                  The printer.device can validate the
 *                  this value using graphics.library
 *                  FindDisplayInfo() if running
 *                  V36 graphics.library.  If valid,
 *                  printer.device queries the graphics.library
 *                  for more information (e.g., aspect ratio).
 *
 *                  This method
 *                  makes it a very simple process for
 *                  V36 developers to take advantage of
 *                  new capabilities in the printer.device
 *                  which allow for proper aspect ratio
 *                  calculation of new graphics modes.
 *                  Plus this allows for a simple upgrade
 *                  path in the future, though it does not
 *                  allow for all possible future graphics
 *                  modes.
 *
 *                  Possible problems:
 *
 *                  Developers who have passed junk in
 *                  'io_modes'.  This would require both
 *                  junk in the upper word, and setting
 *                  the extend bit.  Plus the junk would
 *                  have to match a valid mode in the
 *                  database maintained by graphics.library.
 *
 *
 * Build: Compile on Amiga under Lattice 5.05; if V36 protos, and
 * pragmas available with Lattice release, then some of the inclusion
 * code below wont be needed.  See Carolyn's comments above
 * regarding compilation switches.
 *
 * Set LIB_VERSION36 to TRUE to enable new code.
 *
 * Requires V36 amiga.lib to compile additions successfully.
 *
 */


/*
 * GraphicDump.c  - Carolyn Scheppner  CBM  03/87
 *                Program to do a raster dump of the front (or only)
 *                screen to the graphic printer selected in Preferences.
 *
 * Pure code - can be made resident
 *
 * Modified:  04/17/86  code size optimization
 *            09/10/86  more optimization
 *            11/21/86  added size options
 *            03/30/87  added xdots:ydots option
 *            03/30/88  made reentrant
 *            04/07/90  lattice 5.04-o-sized
 *	36_10 12/26/90  use bumprev, use pragmas, add version string (cas)
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <devices/printer.h>
#include <libraries/dos.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>

#define LIB_VERSION36   1  /* set to 0, or 1 for TRUE */


#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/alib_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

#include "graphicdump_rev.h"

#define TINY      0
#define SMALL     1
#define MEDIUM    2
#define LARGE     3
#define DOTS      4

char *vers=VERSTAG;

struct Library *IconBase;

ULONG fracs[] = {0x55555555,0x80000000,0xAAAAAAAA};

int getWbSize(struct WBStartup *wbMsg, LONG *xDotp, LONG *yDotp);
int getSize(UBYTE *s, LONG *xDotp, LONG *yDotp);
int mystrcmp(UBYTE *a, UBYTE *b);

char Copyright[] =
  "Copyright (c) 1988,1990 Commodore-Amiga,Inc. All Rights Reserved";



void main(int argc,char **argv)
   {

#if LIB_VERSION36

   struct GfxBase *GfxBase;
   ULONG modeID;

#endif

   struct WBStartup *WBenchMsg;
   struct IntuitionBase *IntuitionBase;
   struct IODRPReq *iodrp;
   struct MsgPort  *printerPort;
   struct Screen   *screen;
   struct ViewPort *vp;
   int size;


   LONG xDots=0, yDots=0, error=0;


   if((argc == 2)&&(*argv[1] == '?'))
      {
      Write(Output(),
      "Usage: Graphicdump [ tiny | small | medium | large | xdots:ydots ]\n",
          67);
      exit(RETURN_OK);
      }

   if(argc == 0)
      {
      WBenchMsg = (struct WBStartup *)argv;
      size = getWbSize(WBenchMsg,&xDots,&yDots);
      }
   else if(argc > 1)  size = getSize(argv[1],&xDots,&yDots);
   else size = LARGE;

   /* 
      Let this program run regardless of version, but later we wont
      deal with new graphics modes unless running under >= V36
   */

   if (! (IntuitionBase=
      (struct IntuitionBase *)OpenLibrary("intuition.library",0L)))
         exit(RETURN_FAIL);

   if(printerPort = (struct MsgPort *)CreatePort("gdmp2",0))
      {
      if(iodrp=(struct IODRPReq *)
          CreateExtIO(printerPort,sizeof(struct IODRPReq)))
         {
         if(!(OpenDevice("printer.device",0,iodrp,0)))
            {
            Delay(500);   /* 500/50 = 10 second delay before it starts */

            screen = IntuitionBase->FirstScreen;   /* ptr to front Screen */
            vp = &screen->ViewPort;

            iodrp->io_Command = PRD_DUMPRPORT;
            iodrp->io_RastPort = &screen->RastPort;
            iodrp->io_ColorMap = vp->ColorMap;

            /* We set io_Modes using the default method
               and then try to do it the right way
               using the V36 graphics.library calls
            */


            iodrp->io_Modes = (ULONG)vp->Modes;

            /* special handling for V36 new graphic modes */
            /* extra values in modes field set            */
            /* we really should be doing LockPubScreen()  */
            /* here, but cannot realistically do so since */
            /* the graphicsdump program looks at          */
            /* IBase->FrontScreen which is not our screen */
            /* nor necessarily a public screen            */

#if LIB_VERSION36

            if(GfxBase = (struct Library *)
                OpenLibrary("graphics.library",36L))
            {
               modeID=GetVPModeID(vp);


               if(modeID != INVALID_ID)
               {               

                  iodrp->io_Modes=(ULONG)modeID;

               }
               CloseLibrary(GfxBase);
            }
#endif

            /* iodrp->io_SrcX = 0;     MEMF_CLEAR zeroed this  */
            /* iodrp->io_SrcY = 0;     MEMF_CLEAR zeroed this  */
            iodrp->io_SrcWidth = screen->Width;
            iodrp->io_SrcHeight = screen->Height;
            /* iodrp->io_DestRows = 0; MEMF_CLEAR zeroed this  */

            if(size < LARGE)
               {
               iodrp->io_DestCols = fracs[size];
               iodrp->io_Special = SPECIAL_FRACCOLS|SPECIAL_ASPECT;
               }
            else if(size==DOTS)
               {
               /* iodrp->io_Special = 0L; MEMF_CLEAR zeroed this */ 
               iodrp->io_DestCols = xDots;
               iodrp->io_DestRows = yDots;
               }
            else  /* LARGE */
               {
               iodrp->io_Special = SPECIAL_FULLCOLS|SPECIAL_ASPECT;
               }


            error = DoIO(iodrp);
            CloseDevice(iodrp);
            }
         DeleteExtIO(iodrp);
         }
      DeletePort(printerPort);
      }
   CloseLibrary(IntuitionBase);
   exit(error);
   }

int getWbSize(struct WBStartup *wbMsg, LONG *xDotp, LONG *yDotp)
   {
   struct WBArg  *wbArg;
   struct DiskObject *diskobj;
   UBYTE **toolarray;
   UBYTE *s;
   int size;
   struct Library *iconbase;

   wbArg = wbMsg->sm_ArgList;
   
   size = LARGE;
   if((iconbase = OpenLibrary("icon.library", 0)))
      {
      IconBase = iconbase;
      if(diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name))
         {
         toolarray = diskobj->do_ToolTypes;
         if((s=(UBYTE *)FindToolType(toolarray,"SIZE")))
            {
            size = getSize(s,xDotp,yDotp);
            }
         FreeDiskObject(diskobj);
         }
      CloseLibrary(iconbase);
      }
   return(size);
   }


int getSize(UBYTE *s, LONG *xDotp, LONG *yDotp)
   {
   int size;
   UBYTE *p;

   size = LARGE;
   if (! mystrcmp(s,"tiny"))          size = TINY;
   else if (! mystrcmp(s,"small"))    size = SMALL;
   else if (! mystrcmp(s,"medium"))   size = MEDIUM;
   else
      {
      p = s;
      while((*p)&&(*p != ':')) p++;
      if(*p==':')
         {
         *p = NULL;
         p++;
         *xDotp = atoi(s);
         *yDotp = atoi(p);
         if((*xDotp>0) && (*yDotp>0)) size = DOTS;
         }
      }
   return(size);
   }

   
int mystrcmp(UBYTE *a, UBYTE *b)
   {
   UBYTE *c;
   c = a;
   while(*c) *c |= 0x20, c++;
   while(*a++ == *b)
      {
      if (! *b++) return(0);
      }
   if (*a < *b) return(-1);
   return(1);
   }


/* converts an ascii string into an int */
int atoi( char *s )
{
    int num = 0;
    int neg = 0;

    if( *s == '+' ) s++;
    else if( *s == '-' ) {
        neg = 1;
        s++;
    }

    while( *s >= '0' && *s <= '9' ) {
        num = num * 10 + *s++ - '0';
    }

    while( *s >= '0' && *s <= '9' ) {
        num = num * 10 + *s++ - '0';
    }

    if( neg ) return( - num );
    return( num );
}
