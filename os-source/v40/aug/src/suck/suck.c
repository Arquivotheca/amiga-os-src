/** test program main  **/

#include <exec/types.h>
#include <intuition/intuition.h>
#include "suck.h"

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;

struct  Window      *getNewWind();

ULONG   flg = WINDOWCLOSE | NOCAREREFRESH | WINDOWDRAG | ACTIVATE
		| WINDOWDEPTH | SMART_REFRESH;

ULONG   iflg = CLOSEWINDOW | GADGETDOWN | GADGETUP;

struct ChunkState s;
struct Window   *window = NULL;

main()
{
    struct  IntuiMessage    *msg;
    WORD    exitval = 0;
    BOOL    ready = FALSE;

    /* hold data from *msg  */
    ULONG   class;
    USHORT  code;
    APTR    iaddr;

    if ((IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 0)
	) == NULL)
    {
	printf("NO INTUITION LIBRARY\n");
	exitval = 1;
	goto EXITING;
    }

    if ((GfxBase = 
	(struct GfxBase *) OpenLibrary("graphics.library", 0)
	) == NULL)
    {
	printf("NO GRAPHICS LIBRARY\n");
	exitval = 2;
	goto EXITING;
    }

    /*  init integers */
    cksizesi.LongInt = DEFCHUNKSIZE;
    sprintf(cksizebuf, "%ld", cksizesi.LongInt);
    ckfreesi.LongInt = 0;
    sprintf(ckfreebuf, "%ld", ckfreesi.LongInt);

    /* init libraries and window    */
    window = getNewWind(WLEFT, WTOP, WWIDTH, WHEIGHT, flg, iflg, FIRSTGADGET);

    if (window == NULL)
    {
	printf("test: SysInit failed.\n");
	exitval = 1;
	goto EXITING;
    }

    s.cs_size = DEFCHUNKSIZE;
    s.cs_count = 0;	/* no chunks sucked */

    ready = TRUE;	/* so we know s is valid */

    suckMsg("No chunks sucked");
    availMsg();

    FOREVER
    {
	if ((msg = (struct IntuiMessage *)GetMsg(window->UserPort)) == NULL)
	{
	    Wait(1<<window->UserPort->mp_SigBit);
	    continue;
	}

	class   = msg->Class;
	code    = msg->Code;
	iaddr   = msg->IAddress;
	ReplyMsg(msg);

	switch (class)
	{
	case GADGETDOWN:
	case GADGETUP:
	    doGadg(iaddr);
	    break;

	case CLOSEWINDOW:
	    goto EXITING;

	default:
	    printf("unknown event: class %lx\n", class);
	}
    }

EXITING:
    if (ready)
    {
	chunkFree(&s, FREEALL);	/* free all chunks */
    }
    if (window) CloseWindow(window);
    if (GfxBase) CloseLibrary(GfxBase);
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    /** FIX: FreeRemember() */
    exit (exitval);
}


struct  Window * getNewWind(left, top, width, height, flg, iflg, gadget)
SHORT   left, top, width, height;
ULONG   flg, iflg;
{
    struct  Window  *OpenWindow();
    struct  NewWindow   nw;

    nw.LeftEdge =   (SHORT) left;
    nw.TopEdge  =   (SHORT) top;
    nw.Width    =   (SHORT) width;
    nw.Height   =   (SHORT) height;
    nw.DetailPen    =   (UBYTE) -1;
    nw.BlockPen =   (UBYTE) -1;
    nw.IDCMPFlags   =   (ULONG) iflg;

    nw.Flags    =   (ULONG) flg;

    nw.FirstGadget  =   (struct Gadget *)   gadget;
    nw.CheckMark    =   (struct Image *)    NULL;
    nw.Title    =   (UBYTE *)   " Suck Tool ";
    nw.Screen   =   (struct Screen *)   NULL;
    nw.BitMap   =   (struct BitMap *)   NULL;
    nw.MinWidth =   (SHORT) 50;
    nw.MinHeight=   (SHORT) 30;
    /* work around bug  */
    nw.MaxWidth =   (SHORT) nw.Width;
    nw.MaxHeight    =   (SHORT) nw.Height;
    nw.Type     =   (USHORT) WBENCHSCREEN;

    return ((struct Window *) OpenWindow(&nw));
}
