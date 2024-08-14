
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

/*
 * main.c -- Skeleton program for a typical Commodities application.
 */

#include "local.h"
#include <signal.h>
#include <dos.h>

struct IntuitionBase *IntuitionBase = NULL;
struct Library       *CxBase        = NULL;
struct GfxBase       *GfxBase       = NULL;
struct DosLibrary    *DOSBase       = NULL;
struct Library       *GadToolsBase  = NULL;
struct Library       *IconBase      = NULL;

char     **ttypes;

/* these globals are the connection between the main program
 * loop and the two message handling routines
 */

struct MsgPort *cxport   = NULL; /* commodities messages here      */
ULONG          cxsigflag = 0;    /* signal for above               */

struct MsgPort *iport    = NULL; /* Intuition IDCMP messages here   */
ULONG          isigflag  = 0;    /* signal for above                */

#if CSIGNAL
LONG           csignal   = -1L;
struct Task    *maintask = NULL;
#endif
ULONG          csigflag  = 0L;

char UsageBuf[1024];

VOID main(int,char **);
VOID main( argc, argv )
int  argc;
char **argv;
{
   ULONG    sigrcvd;
   struct   Message    *msg;
   W( char  *str; )

   signal(SIGINT,SIG_IGN); /* Allow SIGBREAKF_CTRL_C to pass through */

   CxBase       =                        OpenLibrary("commodities.library",5);
   DOSBase      =(struct DosLibrary *)   OpenLibrary("dos.library",0);
   IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",33);
   GfxBase      =(struct GfxBase *)      OpenLibrary("graphics.library",0);
   GadToolsBase =                        OpenLibrary("gadtools.library",36);
   IconBase     =                        OpenLibrary("icon.library",33);

#if WINDOW
   if ( ! ( IntuitionBase && CxBase && GfxBase && DOSBase && GadToolsBase && IconBase) )
   {
      D1( kprintf("main.c: main() Failed to open one or more libraries\n") );
      terminate();
   }
#else
   if ( ! ( IntuitionBase && CxBase && DOSBase && GadToolsBase && IconBase) )
   {
      D1( kprintf("main.c: main() Failed to open one or more libraries\n") );
      terminate();
   }
#endif

#if CSIGNAL
   if((csignal = AllocSignal(-1L))==-1)
   {
      D1( kprintf("main.c: main() Failed to get custom signal\n") );
      terminate();
   }
   csigflag = 1L <<csignal;
   maintask=FindTask(0L);
#endif

   if((argc==2)&&(argv[1][0]=='?'))
   {
#if WINDOW
	  sprintf(UsageBuf,"\nUsage: %s [CX_PRIORITY=n] [CX_POPUP=YES|NO] [CX_POPKEY=hotkey] %s\n\n",argv[0],APPARGS);
#else
	  sprintf(UsageBuf,"\nUsage: %s [CX_PRIORITY=n] %s\n\n",argv[0],APPARGS);
#endif
      Write(Output(),UsageBuf,strlen(UsageBuf));
      terminate();
   }

   /* commodities support library function to find argv or tooltypes   */
   ttypes = ArgArrayInit( argc, argv );
   D1(
      {
         int xx=0;

         while(ttypes[xx])
         {
            printf("ttypes[0x%lx]=%s\n",xx,ttypes[xx]);
            xx++;
         }
      }
   )

   if ( ! setupCX( ttypes ) )
   {
      D1( kprintf("main.c: main() setupCX failed\n") );
      terminate();
   }
   W(
      str = ArgString( ttypes, POP_ON_START_TOOL_TYPE,CX_DEFAULT_POP_ON_START);
      if(strcmpi(str,"yes")==0)
         setupWindow();         /* will try to setup iport      */
      D1( kprintf("main.c: main() After setupWindow\n") );
   )

   for (;;)            /* exit by calling terminate   */
   {
      /* handling two ports:
       * either will wake us up;
       * simple approach often works.
       */
      D1( kprintf("main.c: main() Waiting for a signal\n") );
      sigrcvd = Wait ( SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | isigflag | cxsigflag | csigflag );
      D1( kprintf("main.c: main() Recieved a signal\n") );

      /* commodities convention: easy to kill   */
      if (( sigrcvd & SIGBREAKF_CTRL_E )||(sigrcvd&SIGBREAKF_CTRL_C))
      {
         D1( kprintf("main.c: main() Recieved a SIGBREAKF_CTRL_E/C\n") );
         terminate();
      }

#if CSIGNAL
      if ( sigrcvd & csigflag )
      {
         D1( kprintf("main.c: main() Recieved a Custom Signal\n") );
         handleCustomSignal();
      }
#endif

      while(cxport && (msg=GetMsg(cxport)))
         handleCxMsg(msg);
      W(
         while(iport && (msg=(struct Messge *)GT_GetIMsg(iport)))
            handleIMsg((struct IntuiMessage *)msg);
      )
   }

}
/****i* Blank.ld/terminate() ******************************************
*
*   NAME
*        terminate -- Cleanup all resources and exit the program.
*
*   SYNOPSIS
*        terminate()
*
*        VOID terminate(VOID);
*
*   FUNCTION
*        This function performs all the necessary cleanup for the
*        commodity and calls exit() to return control to the OS.
*        No matter how the program exits this should be the last function
*        called.
*
*   INPUTS
*        None.
*
*   RESULT
*        All resources are freed and the program exits.
*
*   EXAMPLE
*        if(!AllocWindow())
*           terminate();
*
*   NOTES
*        This function must be set up so that it can be called at any
*        time regardless of the current state of the program.
*
*   BUGS
*
*   SEE ALSO
*        shutdownCX();
*        shutdownWindow();
*
*****************************************************************************
*
*/
VOID terminate()
{
   D1( kprintf("main.c: terminate() enter\n") );

   shutdownCX();
   D1( kprintf("main.c: terminate(), after shutdownCX()\n") );

   W(
      shutdownWindow();
      D1( kprintf("main.c: terminate(), after shutdownWindow()\n") );
   )

   ArgArrayDone();    /* cx_supp.lib function   */
   D1( kprintf("main.c: terminate(), after ArgArrayDone()\n") );

#if CSIGNAL
   if(csignal != -1) FreeSignal(csignal);
   D1( kprintf("main.c: terminate(), after FreeSignal()()\n") );
#endif

   D1( kprintf("main.c: terminate() CxBase        = %lx\n",CxBase) );
   D1( kprintf("main.c: terminate() IntuitionBase = %lx\n",IntuitionBase) );
   D1( kprintf("main.c: terminate() GfxBase       = %lx\n",GfxBase) );
   D1( kprintf("main.c: terminate() DOSBase       = %lx\n",DOSBase) );
   D1( kprintf("main.c: terminate() GadToolsBase  = %lx\n",GadToolsBase) );
   D1( kprintf("main.c: terminate() IconBase      = %lx\n",IconBase) );

   if(CxBase)        CloseLibrary(CxBase);
   if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
   if(GfxBase)       CloseLibrary((struct Library *)GfxBase);
   if(DOSBase)       CloseLibrary((struct Library *)DOSBase);
   if(GadToolsBase)  CloseLibrary(GadToolsBase);
   if(IconBase)      CloseLibrary(IconBase);

   D1( kprintf("main.c: terminate(), after CloseLibrarys()\n") );

   exit(0);
}
