/* Test program to open the printer device and issue
   a graphics dump command.
   PTEST4 trys to do a SendIO, AbortIO, and a WaitIO.
*/

#define WIDTH 320
#define HEIGHT 50

#include <exec/types.h>
#include <exec/exec.h>
#include <graphics/gfx.h>
#include <graphics/regions.h>
#include <graphics/copper.h>
#include <graphics/gels.h>
#include <intuition/intuition.h>
#include <devices/keymap.h>
#include <devices/printer.h>

struct MsgPort PrtPort; /* a msg port for the printer */
struct IODRPReq PrtIOB; /* the printer IO req block */
struct ViewPort *vp;
struct Window *OpenWindow(), *w;
ULONG IntuitionBase;
int signal;

main()
{
   ULONG err;
   UWORD xorg, yorg, width, height;
   LONG prtwidth, prtheight;

   if ((signal=AllocSignal(-1))<0) {
      printf("can't get a signal\n");
      Panic(0);
   }
   printf("signal=%ld\n",signal);
   Key();
/* open the device */
   if ((err=OpenDevice("printer.device",0,&PrtIOB,0))!=0) {
      printf("open device failed\n");
      Panic(1);
   }
   printf("opendevice err=%ld\n",err);
   Key();
/* setup the msg port in the I/O request */
   PrtPort.mp_Node.ln_Type = NT_MSGPORT;
   PrtPort.mp_Flags = 0;
   PrtPort.mp_SigBit = signal;
   PrtPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
   AddPort(&PrtPort);
   PrtIOB.io_Message.mn_ReplyPort = &PrtPort;
/* open a window to dump */
   if(!OpenAWindow()) {
      printf("can't open a window\n");
      Panic(2);
   }
   printf("window opended\n");
   Key();
/* init vars */
   xorg = yorg = 0;
   width = WIDTH;
   height = HEIGHT;
   prtwidth = 320;
   prtheight = 80;

   vp = &(w->WScreen->ViewPort);
   printf("w=%ld, VP=%ld, RP=%ld, CM=%ld, M=%ld\n"
      ,w,vp,w->RPort,vp->ColorMap,vp->Modes);
/* setup the command */
   PrtIOB.io_Command = PRD_DUMPRPORT;
   PrtIOB.io_RastPort = w->RPort;
   PrtIOB.io_ColorMap = vp->ColorMap;
   PrtIOB.io_Modes = vp->Modes;
   PrtIOB.io_SrcX = xorg;
   PrtIOB.io_SrcY = yorg;
   PrtIOB.io_SrcWidth = width;
   PrtIOB.io_SrcHeight = height;
   PrtIOB.io_DestCols = prtwidth;
   PrtIOB.io_DestRows = prtheight;
   PrtIOB.io_Special = 0;
/* do it */
   Key();
printf("calling SendIO\n");
   SendIO(&PrtIOB);
printf("out of SendIO...");
printf("Get ready to abort\n");
   Key();
   AbortIO(&PrtIOB);
printf("aborted, now waiting...");
   WaitIO(&PrtIOB);
printf("ok\n");
/* check for an error */
   if ((err=PrtIOB.io_Error)!=0) {
      printf("I/O error # %ld\n",err);
      Panic(3);
   }
/* shut down */
   printf("shutting down\n");
   Panic(3);
}
OpenAWindow()
{
   if((IntuitionBase=OpenLibrary("intuition.library",0))==0) {
      printf("can't open intuition\n");
      return(FALSE);
   }
   {
   struct NewWindow nw;
   nw.LeftEdge = 0;
   nw.TopEdge = 0;
   nw.Width = WIDTH;
   nw.Height = HEIGHT;
   nw.DetailPen = -1;
   nw.BlockPen = -1;
   nw.IDCMPFlags = NULL;
   nw.Flags = WINDOWDRAG|WINDOWDEPTH|SMART_REFRESH;
   nw.FirstGadget = NULL;
   nw.CheckMark = NULL;
   nw.Title = "Printer Test";
   nw.Screen = NULL;
   nw.BitMap = NULL;
   nw.MinWidth = nw.Width;
   nw.MinHeight = nw.Height;
   nw.MaxWidth = nw.Width;
   nw.MaxHeight = nw.Height;
   nw.Type = WBENCHSCREEN;
   w = OpenWindow(&nw);
   }
   if (w==0) return(FALSE);
   else return(TRUE);
}
Panic(code)
int code;
{
   if (code>=3) CloseWindow(w);
   if (code>=2) {
      CloseDevice(&PrtIOB);
      RemPort(&PrtPort);
   }
   if (code>=1) FreeSignal(signal);
   exit();
}
Key()
{
int c;
   printf("hit return...");
   c = getchar();
}
