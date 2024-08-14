#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <ctype.h>
#include <string.h>
#include <libraries/gadtools.h>
#include <utility/tagitem.h>
#include        <clib/alib_protos.h>
#include        <clib/asl_protos.h>
#include        <clib/dos_protos.h>
#include        <clib/exec_protos.h>
#include        <clib/gadtools_protos.h>
#include        <clib/graphics_protos.h>
#include        <clib/intuition_protos.h>

struct IntuitionBase *IntuitionBase ;
struct GfxBase       *GfxBase ;
struct Library       *GadToolsBase ;

STATIC struct NewWindow nw = {
        0, 0, 640, 200, -1, -1, 
        0L,                                            /* IDCMP */
        WINDOWDRAG|WINDOWDEPTH|ACTIVATE|WINDOWCLOSE,   /* flags */
        0, 0, NULL, 0, 0, 640, 100, -1, -1, 
        CUSTOMSCREEN,
        };

                /* this is the screen used if screen mode is selected */
STATIC struct ExtNewScreen scrdef = { 0,0,0,0,
        2,      /* bit planes */
        0,1,    /* pens */
        HIRES|LACE,
        CUSTOMSCREEN | NS_EXTENDED,
        NULL,NULL,NULL,NULL
        };

struct TagItem screentags[] = {
	{ SA_DClip, 0L } ,
	{ SA_DisplayID, 0x41000 },
	{ SA_AutoScroll, TRUE },
	{ TAG_DONE, 0L }
	} ;


struct Rectangle *rect ;
struct DisplayInfo displayinfo ;
struct DimensionInfo dimensioninfo ;
struct IBox *ibox ;


main()
{
	struct Screen *wbscreen, *myscreen ;
	struct Window *win ;
	struct Rectangle r ;
	
	r.MinX = r.MinY = 0 ;
	
	if( IntuitionBase = (struct IntuitionBase*)OpenLibrary("intuition.library", 36L))
	{
		if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 36L))
		{
			if(GadToolsBase = OpenLibrary("gadtools.library", 36L))
			{
				if(wbscreen = LockPubScreen(NULL))
				{
					r.MaxX = scrdef.Width  = wbscreen->Width ;
					r.MaxY = scrdef.Height = wbscreen->Height ;
					screentags[0].ti_Data = (LONG)&r ;

					scrdef.Extension = &screentags[0] ;
					if(myscreen = OpenScreen((struct NewScreen *)&scrdef))
					{
						printf("screen opened!\n") ;
						printf("width  = %ld\n", myscreen->Width ) ;
						printf("height = %ld\n", myscreen->Height ) ;
						nw.Screen = myscreen ;
						if(win = OpenWindow(&nw))
						{
							printf("window opened\n") ;
							Delay(500L) ;
							CloseWindow(win) ;
						}
						CloseScreen(myscreen) ;
					}
					else
					{
						printf("screen failed to open\n") ;
					}
					UnlockPubScreen(NULL, wbscreen) ;
				}
				CloseLibrary(GadToolsBase) ;
			}
			CloseLibrary((struct Library *)GfxBase) ;
		}
		CloseLibrary((struct Library *)IntuitionBase) ;
	}
}
