#include    "exec/types.h"
#include    "intuition/intuition.h"
       
#define     WWIDTH  480
#define     WHEIGHT 11

ULONG IntuitionBase;
  
struct NewWindow nw = {
    50, 50, WWIDTH, WHEIGHT,
    1, 2, CLOSEWINDOW, SMART_REFRESH | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE,
    0, 0, "Parallel Port allocated to Sidecar LPT1 ", 0, 0,
    WWIDTH, WHEIGHT, WWIDTH, WHEIGHT, WBENCHSCREEN
};

main()
{
    struct Window *w;

    if ((IntuitionBase = OpenLibrary("intuition.library", 0)) == 0) {
	exit(20);
    }
    if (OpenLPT1()) {
	w = (struct Window *) OpenWindow(&nw);
	if (w == 0) {
	    CloseLPT1();
	    CloseLibrary(IntuitionBase);
	    exit(20); 
	}
	Wait(1<<(w->UserPort->mp_SigBit));
	CloseLPT1();
	CloseWindow(w);
    }
    CloseLibrary(IntuitionBase);
}