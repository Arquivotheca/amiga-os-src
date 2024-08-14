/* democlasslib.c -- demonstration of 		:ts=8
 * using a public image class from a library.
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

#define D(x)	x

struct  IntuitionBase   *IntuitionBase;
struct Library	*MyClassBase;

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

    D( printf("got myclass.library at %lx\n", MyClassBase ) );

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

    cleanup( "all done, disposing image before closelibrary." );
}

/*
 * try to get the system to flushlibs
 */
inciteExpunge()
{
    void *mem;

#define LOTS_O_MEM	(2000000L)

    if ( mem = (void *) AllocMem( LOTS_O_MEM, 0 ) )
    {
	D( printf("GOT MEM %lx!\n", mem ) );
    	FreeMem( mem, LOTS_O_MEM );
    }
}

cleanup( str )
char	*str;
{
    if (str) printf("%s\n", str);

    if (IntuitionBase) CloseLibrary(IntuitionBase);

    DisposeObject( myimage );
    D( printf("have disposed image.\n") );

    D( printf("close class library %lx\n", MyClassBase ) );
    if (MyClassBase) CloseLibrary(MyClassBase);

    D( printf("expunge ... ") );
    inciteExpunge();
    D( printf("done\n") );

    exit (0);
}

/* exits via cleanup() if failure	*/
openAll()
{
    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(MyClassBase=(struct Library *)OpenLibrary("myclass.library", 36L)))
    { cleanup("no myclass.library\n"); }
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
