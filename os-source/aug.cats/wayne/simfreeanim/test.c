
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <stdio.h>
#include <stdlib.h>

#include "fakefreeanim_pragmas.h"

int __saveds StartFakeAnim( VOID );

struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* FreeAnimBase;

#define	MAXPNTS	100

VOID
closedown( VOID )
{

    if ( FreeAnimBase )
	CloseLibrary( (struct Library *)FreeAnimBase );

    if ( IntuitionBase )
	CloseLibrary( (struct Library *)IntuitionBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    exit(RETURN_OK);

} // closedown()


VOID
init( VOID )
{

    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)) )
	closedown();

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0)))
	closedown();

    if(!(FreeAnimBase = (struct Library *)OpenLibrary("freeanim.library",40L))) {
	printf("Could NOT open freeanim.library\n");
	closedown();
    }

} // init()


main()
{
    struct Screen		* screen;
    struct Window		* win;
    struct IntuiMessage		* imsg;
    ULONG			  width,height;
    BOOL			  done = FALSE;

    init();

    if ( screen = (struct Screen *)OpenScreenTags(NULL,
	SA_Width,	310,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_DisplayID,	NTSC_MONITOR_ID|LORES_KEY,
	TAG_DONE) )
    {

	if ( win = (struct Window *)OpenWindowTags(NULL,
	    WA_Width,		screen->Width,
	    WA_Height,		screen->Height,
	    WA_IDCMP,		IDCMP_MOUSEBUTTONS,
	    WA_Flags,		BACKDROP | BORDERLESS | RMBTRAP,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	screen,
	    TAG_DONE) )
	{
	    width = GetBitMapAttr( win->RPort->BitMap, BMA_WIDTH );
	    height = GetBitMapAttr( win->RPort->BitMap, BMA_HEIGHT );

	    StartFakeAnim();


    Delay( 100 );
    if((FreeAnimBase = (struct Library *)OpenLibrary("freeanim.library",40L))) {
	Delay( 100 );
	CloseLibrary( (struct Library *)FreeAnimBase );
    }

	    while( !done ) {
		Wait(1 << win->UserPort->mp_SigBit);
		while ( imsg = (struct IntuiMessage *)GetMsg(win->UserPort) ) {
		    if ( (imsg->Class == MOUSEBUTTONS) && (imsg->Code == MENUDOWN) )
			done = TRUE;
		    ReplyMsg( (struct Message *)imsg );
		}
	    }

	    CloseWindow ( win );
	}

	CloseScreen( screen );
    }

    closedown();

} // main()



