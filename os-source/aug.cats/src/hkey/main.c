/*
 * main.c -- Skeleton program for a typical Commodities application.
 */

#include "mycxapp.h"

extern LONG __asm WB2CLI (register __a0 struct WBStartup * wbmsg,
                    register __d0 ULONG DefaultStack,
                    register __a1 struct Library * DOSBase);


struct IntuitionBase *IntuitionBase = NULL;
struct Library       *CxBase        = NULL;
struct GfxBase       *GfxBase       = NULL;
struct Library       *GadToolsBase  = NULL;
struct Library 	     *IconBase      = NULL;

char     **ttypes;

/* these globals are the connection between the main program
 * loop and the two message handling routines
 */

struct MsgPort *cxport   = NULL; /* commodities messages here      */
ULONG          cxsigflag = 0;    /* signal for above               */

struct MsgPort *iport    = NULL; /* Intuition IDCMP messages here   */
ULONG          isigflag  = 0;    /* signal for above                */

BPTR   mycd = NULL, oldcd = NULL;

void main(int argc,char ** argv)
{
   extern struct DOSLibrary *DOSBase;
   ULONG    sigrcvd;
   struct   Message    *msg;
   W( char  *str; )

   extern struct DosLibrary *DOSBase;

   D1( kprintf("hkey main: about to open libraries\n") );

   CxBase       =                        OpenLibrary("commodities.library",5);
   IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",33);
   GfxBase      =(struct GfxBase *)      OpenLibrary("graphics.library",0);
   GadToolsBase =                        OpenLibrary("gadtools.library",36);
   IconBase =                            OpenLibrary("icon.library",36);

   if ( ! ( IntuitionBase && CxBase && GfxBase && GadToolsBase && IconBase) )
   {
      D1( kprintf("main: Failed to open one or more libraries\n") );
      terminate();
   }

   D1( kprintf("main: CxBase        = %lx\n",CxBase) );
   D1( kprintf("main: IntuitionBase = %lx\n",IntuitionBase) );
   D1( kprintf("main: GfxBase       = %lx\n",GfxBase) );
   D1( kprintf("main: GadToolsBase  = %lx\n",GadToolsBase) );
   D1( kprintf("main: IconBase      = %lx\n",IconBase) );
   D1( kprintf("main: DOSBase       = %lx\n",DOSBase) );

   /* commodities support library function to find argv or tooltypes   */
   D1( kprintf("main: before ArgArrayInit\n") );
   if ( ! (ttypes = ArgArrayInit( argc, argv )))
   {
      D1( kprintf("main: ArgArrayInit failed\n") );
      terminate();
   }

   if(!argc) WB2CLI((struct WBStartup *)argv,
                ((struct Process *)FindTask(NULL))->pr_StackSize,
			(struct DosLibrary *)DOSBase);

   D1( kprintf("main: before setupCX\n") );

   if ( ! setupCX( ttypes ) )
   {
      D1( kprintf("main: setupCX failed\n") );
      terminate();
   }

   D1( kprintf("main: before setupWindow\n") );
   W(
      str = ArgString( ttypes, POP_ON_START_TOOL_TYPE,CX_DEFAULT_POP_ON_START);
      if(strcmpi(str,"yes")==0)
         setupWindow();         /* will try to setup iport      */
      D1( kprintf("main: After setupWindow\n") );
   )


   mycd = Lock("SYS:",SHARED_LOCK);
   if(mycd)	CurrentDir(mycd);

   for (;;)            /* exit by calling terminate   */
   {
      /* handling two ports:
       * either will wake us up;
       * simple approach often works.
       */
      D1( kprintf("main: Waiting for a signal\n") );
      sigrcvd = Wait ( SIGBREAKF_CTRL_E | isigflag | cxsigflag );
      D1( kprintf("main: Recieved a signal\n") );

      /* commodities convention: easy to kill   */
      if ( sigrcvd & SIGBREAKF_CTRL_E )
      {
         D1( kprintf("main: Recieved a SIGBREAKF_CTRL_E\n") );
         terminate();
      }

      while(cxport && (msg=GetMsg(cxport)))
         handleCxMsg(msg);
      W(
         while(iport && (msg=(struct Messge *)GT_GetIMsg(iport)))
            handleIMsg((struct IntuiMessage *)msg);
      )
   }

}

VOID terminate()
{
   D1( kprintf("main: terminate() enter\n") );

   shutdownCX();
   D1( kprintf("main: terminate(), after shutdownCX()\n") );

   W(
      shutdownWindow();
      D1( kprintf("main: terminate(), after shutdownWindow()\n") );
   )

   ArgArrayDone();    /* cx_supp.lib function   */
   D1( kprintf("main: terminate(), after ArgArrayDone()\n") );

   D1( kprintf("main: CxBase        = %lx\n",CxBase) );
   D1( kprintf("main: IntuitionBase = %lx\n",IntuitionBase) );
   D1( kprintf("main: GfxBase       = %lx\n",GfxBase) );
   D1( kprintf("main: GadToolsBase  = %lx\n",GadToolsBase) );

   if(oldcd && mycd)	CurrentDir(oldcd);

   if(CxBase)        CloseLibrary(CxBase);
   if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
   if(GfxBase)       CloseLibrary((struct Library *)GfxBase);
   if(GadToolsBase)  CloseLibrary(GadToolsBase);
   if(IconBase)      CloseLibrary(IconBase);

   D1( kprintf("main: terminate(), after CloseLibrarys()\n") );

   exit(0);
}
