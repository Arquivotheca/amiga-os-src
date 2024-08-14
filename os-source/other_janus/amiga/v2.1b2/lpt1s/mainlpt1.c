#include    "exec/types.h"
#include    "intuition/intuition.h"
       
#define     WWIDTH  480
#define     WHEIGHT 11

ULONG IntuitionBase;
ULONG JanusBase;

struct NewWindow nw = {
    50, 50, WWIDTH, WHEIGHT,
    1, 2, CLOSEWINDOW, SMART_REFRESH | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE,
    0, 0, "Parallel Port allocated to Emulator LPT1 V1.3S", 0, 0,
    WWIDTH, WHEIGHT, WWIDTH, WHEIGHT, WBENCHSCREEN
};

main()
{
   struct Window *w;
   int ReturnCode = 20;   /* assume something will fail */

   if (IntuitionBase = OpenLibrary("intuition.library",33))
   {
      if (OpenLPT1())
      {
         if (w = (struct Window *) OpenWindow(&nw))
         {
            ReturnCode=0;    /* assumed wrong, everything worked OK */
            Wait(1<<(w->UserPort->mp_SigBit));
            CloseWindow(w);
         }
         CloseLPT1();
      }
      CloseLibrary(IntuitionBase);
   }
   return(ReturnCode);         /* return 0 or 20 */
}
