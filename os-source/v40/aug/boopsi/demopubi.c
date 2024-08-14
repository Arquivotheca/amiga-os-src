/* demopubi.c -- demonstration of 		:ts=8
 * public image class.
 */

/*
Copyright (c) 1989, 1990 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"

#define D(x)	;

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct  Library         *UtilityBase;

/* class pointer is an abstract handle, which
 * one never uses for "public" access to a class,
 * but we as class owner can use for private access,
 * or to free the class.
 */
void	*initEmbBClass();
void	*EmbBClass;

Object	*NewObject();

ULONG	myiflags = CLOSEWINDOW;

struct Image	*myimage;

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct Window 	*OpenWindowTags();	/* in varargs.c for now */
    struct Window	*w;
    struct DrawInfo	*drinfo;

    openAll();	/* get libraries open	*/

    D( printf("about to openwindow\n") );
    w = OpenWindowTags( NULL, 
		WA_Title,	"Public Image Test Window",
		WA_SimpleRefresh, TRUE,
		WA_NoCareRefresh, TRUE,
		WA_DepthGadget,	TRUE,
		WA_DragBar,	TRUE,
		WA_Left,	300,
		WA_Top,		50,
		WA_Width,	280,
		WA_Height,	120,
		WA_IDCMP,	myiflags,
		WA_CloseGadget,	TRUE,
    		TAG_END );
    D( printf("window at %lx\n ", w ) );
    if ( w == NULL) cleanup( "couldn't open my window.\n");

    drinfo = GetScreenDrawInfo( w->WScreen );

    /* init the public class	*/
    EmbBClass = initEmbBClass();

    /* create an image from my public class	*/
    myimage =  (struct Image *) NewObject(  NULL,  "emboxclass",
			IA_WIDTH, 20,
			IA_HEIGHT, 10,
    			TAG_END );

#define XPOS	(40)
#define YPOS	(20)
#define XPOS2	(XPOS + 30)

    /* draw the image	*/
    DrawImageState( w->RPort, myimage, XPOS, YPOS, IDS_NORMAL, drinfo );

    /* draw the image	*/
    DrawImageState( w->RPort, myimage, XPOS2, YPOS, IDS_SELECTED, drinfo );

    /* go away and be done	*/
    goHandleWindow( w );

    FreeScreenDrawInfo( w->WScreen, drinfo );
    CloseWindow( w );

    DisposeObject( myimage );
    D( printf("have disposed.\n") );

    /* get rid of the public class.
     * Don't really exit unless the class can be free'd,
     * since that would mean that somebody might want to use
     * your class's implementation routines after you're gone.
     */
    if ( ! freeEmbBClass( EmbBClass ) )
    {
	cleanup( "PANIC: exiting with class not free'd!\n" );
    }

    cleanup( "all done" );
}

cleanup( str )
char	*str;
{
    if (str) printf("%s\n", str);

    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (GfxBase) CloseLibrary(GfxBase);
    if (UtilityBase) CloseLibrary(UtilityBase);

    exit (0);
}

/* exits via cleanup() if failure	*/
openAll()
{
    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }
}

goHandleWindow( w )
struct Window	*w;
{
    struct IntuiMessage *imsg;
    for(;;)
    {
	WaitPort( w->UserPort );
	while ( imsg = (struct IntuiMessage *) GetMsg( w->UserPort ) )
	{
	    switch ( imsg->Class )
	    {
	    case CLOSEWINDOW:
	    	ReplyMsg( (struct Message *) imsg );
		return;

	    default:
		D( printf("unknown message \n", imsg->Class));
		break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }
}
