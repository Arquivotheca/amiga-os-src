/* demoframe.c -- demonstration of 		:ts=8
 * "framed" image class.
 */

/*
Copyright (c) 1989 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"

#define D(x)	x

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct  Library         *UtilityBase;

void	*Frame1Class = NULL;
void	*initFrame1Class();

/* from varargs.c -- interface to NewObjectA()	*/
Object	*NewObject();

ULONG	myiflags = CLOSEWINDOW;

struct Image	*frameimage;

main()
{
    struct DrawInfo	*GetScreenDrawInfo();
    struct Window 	*OpenWindowTags();	/* in varargs.c for now */
    struct Window	*w;
    struct DrawInfo	*drinfo;
    struct IBox		srcbox;
    struct IBox		framebox;

    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }

    D( printf("about to openwindow\n") );
    w = OpenWindowTags( NULL, 
		WA_Title,	"frameimage Test Window",
		WA_SimpleRefresh, TRUE,
		WA_NoCareRefresh, TRUE,
		WA_DepthGadget,	TRUE,
		WA_DragBar,	TRUE,
		WA_Left,	300,
		WA_Top,		150,
		WA_Width,	280,
		WA_Height,	120,
		WA_IDCMP,	myiflags,
		WA_CloseGadget,	TRUE,
    		TAG_END );
    D( printf("window at %lx\n ", w ) );

    if ( w == NULL) cleanup( "couldn't open my window.\n");
    drinfo = GetScreenDrawInfo( w->WScreen );

    /* create a frame image which can be shared by contents
     * of different sizes
     */
    Frame1Class = initFrame1Class();
    frameimage =  (struct Image *) NewObject( Frame1Class, NULL, TAG_END );

    D( printf("init'd fram1class: %lx, image %lx\n", Frame1Class, frameimage ));

    /* fake up some contents for the frame	*/
    getContentsBox( &srcbox );

    /* get the dimensions of the enclosing frame, and the relative
     * offset of the enclosing frame
     */
    DoMethod( frameimage, IM_FRAMEBOX, &srcbox, &framebox, drinfo, 0 );

    printf("contents box %d/%d/%d/%d\n",
    		srcbox.Left,
    		srcbox.Top,
    		srcbox.Width,
    		srcbox.Height );

    printf("frame box %d/%d/%d/%d\n",
    		framebox.Left,
    		framebox.Top,
    		framebox.Width,
    		framebox.Height );

#define XPOS	(40)
#define YPOS	(20)

    /* now draw the frame and the properly centered context	*/
    DoMethod( frameimage, IM_DRAWFRAME, 
		    	  w->RPort, 
			  (XPOS<<16)+YPOS,	/* packed position	*/
			  IDS_NORMAL,		/* state		*/
			  drinfo,
			  (framebox.Width << 16)+framebox.Height );

    /* note negative offset of contents, relative to frame	*/
    drawContents( w->RPort, XPOS-framebox.Left, YPOS-framebox.Top );

    goHandleWindow( w );

    FreeScreenDrawInfo( w->WScreen, drinfo );
    CloseWindow( w );

    DisposeObject( frameimage );
    freeFrame1Class( Frame1Class );

    D( printf("have disposed.\n") );
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

struct IBox contentsbox = { 3,7,10,5 };

getContentsBox( box )
struct IBox	*box;
{
    *box = contentsbox;
}

drawContents( rp, xoffset, yoffset )
struct RastPort	*rp;
{
    SetAPen( rp, 3 );
    RectFill( rp,
    	xoffset + contentsbox.Left,
	yoffset + contentsbox.Top,
	xoffset + contentsbox.Left + contentsbox.Width -1,
	yoffset + contentsbox.Top + contentsbox.Height -1 );
}
