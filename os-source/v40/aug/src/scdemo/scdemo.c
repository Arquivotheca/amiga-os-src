/* Screen Demo Program	 :ts=8 */


/*
Copyright (c) 1989 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"


void
NewList( lh )
struct List	*lh;
{
    lh->lh_Tail = NULL;
    lh->lh_TailPred = (struct Node *) lh;
    lh->lh_Head = ( struct Node * ) &(lh)->lh_Tail;
}

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;

struct List	ScreenList;		/* keep track of open screens	*/

struct ScreenNode	{
    struct Node	sn_Node;	
    struct Screen *sn_Screen;
};

struct Screen *addScreen();

#if 0 
don't need these with lattice
struct Node	*RemHead();
struct Screen	*OpenScreen();
struct TextAttr SafeFont = { (UBYTE *) "topaz.font", 8, 0, 0, };
#endif

char	kbuff[257];

showHelp()
{
    printf("commands:\n");
    printf("f - free screens        q - quit\n");
    printf("l - lo-res              L - lo-res interlace\n");
    printf("h - hi-res              H - hi-res interlace\n");
    printf("p - productivity        P - productivity interlace\n");
    printf("s - superhires          S - superhires interlace\n");
    printf("o - std overcan hires   O - video overscan hires\n");
    printf("a - A2024 10 Hz         A - A2024 15 Hz\n");
    printf("w - wide (scrolling) hi res\n");
    printf("z - VGA lores (70ns pixels)\n");
    printf("$ - hex display mode specifier\n");
}


main()
{
    printf("SCDemo: accompanies V1.4 alpha 15.\n");

    NewList( &ScreenList );

    /* not all library versions work with all screens	*/
    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }

    FOREVER
    {
	printf("screen command (? for list): ");
	if (!gets(kbuff)) cleanup("eof.\n");

	switch ( *kbuff )
	{
	case 'o':
	    addScreen( 0xf00, 0x555,
	    	S_DISPLAYID, HIRES_KEY,
		S_STDDCLIP, OSCAN_STANDARD,
		S_TITLE, " Hires Standard Overscan ",
		TAG_DONE );
	    break;

	case 'O':
	    addScreen( 0xf00, 0x555,
	    	S_DISPLAYID, HIRES_KEY,
		S_STDDCLIP, OSCAN_VIDEO,
		S_TITLE, " Hires Video Overscan ",
		TAG_DONE );
	    break;

	case 'a':
	    addScreen( 0xfff, 0x000,
	    	S_DISPLAYID, A2024TENHERTZ_KEY,
		S_TITLE, " A2024, 10 Hz ",
		TAG_DONE );
	    break;

	case 'A':
	    addScreen( 0x000, 0xfff,
	    	S_DISPLAYID, A2024FIFTEENHERTZ_KEY,
		S_TITLE, " A2024, 15 Hz ",
		TAG_DONE );
	    break;

	case 's':
	    addScreen( 0x005, 0xff0,
	    	S_DISPLAYID, SUPER_KEY,
		S_TITLE, " Super-Hires, Non-interlaced ",
		TAG_DONE );
	    break;

	case 'S':
	    addScreen( 0x005, 0xff0,
	    	S_DISPLAYID, SUPERLACE_KEY,
		S_TITLE, " Super-Hires, Interlaced ",
		TAG_DONE );
	    break;

	case 'p':
	    addScreen( 0xfff, 0x000,
	    	S_DISPLAYID, VGAPRODUCT_KEY,
		S_TITLE, " Productivity Mode ",
		TAG_DONE );
	    break;

	case 'P':	/* productivity, lace	*/
	    addScreen( 0x005, 0xf00,
	    	S_DISPLAYID, VGAPRODUCTLACE_KEY,
		S_TITLE, " Productivity, Interlaced ",
		TAG_DONE );
	    break;

	case 'z':
	    addScreen( 0xfff, 0x000,
	    	S_DISPLAYID, VGALORES_KEY,
		S_TITLE, " 70ns pixels, Doublescan ",
		TAG_DONE );
	    break;

	case 'h':
	    addScreen( 0x0f0, 0x555,
	    	S_DISPLAYID, HIRES_KEY,
		S_TITLE, " HiRes Text Overscan ",
		TAG_DONE );
	    break;

	case 'H':
	    addScreen( 0xf00, 0x555,
	    	S_DISPLAYID, HIRESLACE_KEY,
		S_TITLE, " HiRes Interlaced ",
		TAG_DONE );
	    break;

	case 'L':
	    addScreen( 0x0ff, 0x555,
	    	S_DISPLAYID, LORESLACE_KEY,
		S_TITLE, " LoRes Lace Screen ",
		TAG_DONE );
	    break;

	case 'l':
	    addScreen( 0x00f, 0x555,
	    	S_DISPLAYID, LORES_KEY,
		S_TITLE, " LoRes Screen ",
		TAG_DONE );
	    break;

 	case 'w':
	    addScreen( 0xf0f, 0x555,
	    	S_DISPLAYID, HIRES_KEY,
		S_TITLE, " Wide Scrolling HiRes Screen ",

		S_WIDTH, 1100,
		S_AUTOSCROLL, TRUE,

		TAG_DONE );
	    break;

	case 'f':
	    freeScreens();
	    break;

	case '?':
	    showHelp();
	    break;

	case 'q':
	case 'Q':
	    cleanup("bye.\n");
	    break;
	case '$':
	    {
	    	long	scanmode;
		int	scancount;

		/* skip '$' and get long hex value */
		scancount = sscanf( kbuff + 1, "%lx", &scanmode );
		if ( scancount == 1 )
		{
		    sprintf( kbuff,"Direct Mode Specification: %lx",scanmode);
		    addScreen( 0x0ff, 0x555,
		    	S_DISPLAYID, scanmode );
		    break;
		}
		else
		{
		    printf("couldn't parse: type '$ <hex num>'\n");
		}
	    }
	    break;
	default:
	    printf("don't know (%lx)\n", *kbuff, *kbuff);
	}
    }
}

cleanup( str )
char	*str;
{
    if (str) printf(str);

    freeScreens();

    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (GfxBase) CloseLibrary(GfxBase);

    exit (0);
}

/* free all screens	*/
freeScreens()
{
    struct ScreenNode *sn;

    while (sn = (struct ScreenNode *) RemHead( &ScreenList ))
    {
	CloseWindow( sn->sn_Screen->FirstWindow );
	printf("freeing screen at %lx.\n", sn->sn_Screen );
	CloseScreen( sn->sn_Screen );
	FreeMem( sn, (long) sizeof *sn );
    }
}

char	*string = "Test String A B b CdEf m M n N o d D";

struct ColorSpec colorspec[] = {
    { 0 },
    { 1 },
    {-1 }
};

#define REDPART( color )	(( (color) >> 8 ) & 0xf )
#define GREENPART( color )	(( (color) >> 4 ) & 0xf )
#define BLUEPART( color )	(  (color) & 0xf )

struct Screen *
addScreen( color0, color1, tags )
int	color0, color1;
ULONG	tags;
{
    struct ScreenNode *sn;
    struct Screen	*OpenScreenTags();
    struct Window	*OpenWindowTags();
    struct Window	*window;
    ULONG	errorcode;	/* for OpenScreen() errors	*/

    if (!(sn = (struct ScreenNode *)
    	  AllocMem((LONG)sizeof (struct ScreenNode), 0L )) )
    	return (NULL);

    colorspec[0].Red = REDPART( color0 );
    colorspec[0].Green = GREENPART( color0 );
    colorspec[0].Blue = BLUEPART( color0 );

    colorspec[1].Red = REDPART( color1 );
    colorspec[1].Green = GREENPART( color1 );
    colorspec[1].Blue = BLUEPART( color1 );

    sn->sn_Screen =  OpenScreenTags( NULL,
    			S_COLORS, colorspec,
			S_ERRORCODE, &errorcode,
			TAG_MORE, &tags,	/* jump to varargs list */
			TAG_DONE );

    if ( sn->sn_Screen )
    {
	window = OpenWindowTags( NULL,
		W_CUSTOMSCREEN, sn->sn_Screen,
		W_TITLE, " Window in Screen ",
		TAG_DONE );

	printf("window at %lx\n", window );

        if ( window )	/* i.e., success */
        {
	    printf("new screen at %lx.\n", sn->sn_Screen);
	    AddTail(&ScreenList, (struct Node *) sn);

	    /* draw a little something at the bottom	*/
	    drawRulers( sn->sn_Screen );
	    return (sn->sn_Screen);
        }
	else
	{
	    printf( "couldn't get window for screen\n" );
	    CloseScreen( sn->sn_Screen );
	}
    }

    else
    {
	printf("can't open screen: ");
    	switch ( errorcode )
	{
	case OSERR_NOMONITOR:
	    printf("monitor not available.\n");
	    break;
	case OSERR_NOCHIPS:
	    printf("new chipset not available.\n");
	    break;
	case OSERR_NOMEM:
	    printf("memory not available.\n");
	    break;
	case OSERR_NOCHIPMEM:
	    printf("chip memory not available.\n");
	    break;
	case OSERR_PUBNOTUNIQUE:
	    printf("public screen already open.\n");
	    break;
	case OSERR_UNKNOWNMODE:
	    printf("mode ID is unknown.\n");
	    break;
	default:
	    printf("unknown error %ld\n", errorcode );
	}
    }
    FreeMem(sn, (LONG) sizeof (struct ScreenNode));
    return (NULL);
}

#define HRULEY		(90L)
#define VRULEX		(250L)
#define VRULESTART	(1)			/* start VRULE at 200	*/

#define HASHHEIGHT	(20L)		/* size of hash marks	*/
#define HASHWIDTH	(20L)	

/*
 * draw rulers in screen
 */
UBYTE	posbuff[ 81 ];

drawRulers( sc )
struct Screen *sc;
{
#if 0
    struct RastPort *rp = sc->FirstWindow->RPort;
#else
    struct RastPort *rp = &sc->RastPort;
#endif
    int		hunnert;
    int		pos;
    int		endpos;
    int		tlength;
    int		width = sc->Width;
    int		height = sc->Height;

    /* report total dimensions	*/
    sprintf( posbuff, "Left %d, Top %d, Width %d, Height %d.",
    	sc->LeftEdge, sc->TopEdge, sc->Width, sc->Height );
    SetAPen( rp, 1L );
    Move( rp, 10L, (LONG) HRULEY - 50 );
    Text( rp, posbuff, (LONG) strlen( posbuff ) );


    /* draw horizontal counters and hashmarks */
    SetAPen( rp, 1L );
    Move( rp, 0L, (LONG) HRULEY + HASHHEIGHT / 2 );
    Draw( rp, (LONG) width - 1, (LONG) HRULEY + HASHHEIGHT / 2 );

    for (hunnert = 0; (hunnert * 100) <= width; ++hunnert)
    {
	/* tall hashmark at X00	*/
	pos = hunnert * 100;
	endpos = pos + 100;
	SetAPen( rp, 3L );
	Move( rp, (LONG) pos, HRULEY );
	Draw( rp, (LONG) pos, HRULEY  + 2 * HASHHEIGHT);

	while (  (pos += 10) < endpos )
	{
	    if ( pos > (width - 1) ) goto BREAK1;
	    Move( rp, (LONG) pos, HRULEY );
	    Draw( rp, (LONG) pos, HRULEY  + HASHHEIGHT);
	}

	sprintf( posbuff, "%d", endpos);
	tlength = TextLength( rp, posbuff, (LONG) strlen( posbuff ) );
	SetAPen( rp, 7L );
	Move( rp, (LONG) endpos - tlength - 1,
	HRULEY  + 2 * HASHHEIGHT);
	Text( rp, posbuff, (LONG) strlen( posbuff ) );
    }
BREAK1:

    /* draw vertical counters */
    Move( rp, VRULEX + HASHWIDTH / 2, (LONG) VRULESTART * 100 );
    Draw( rp, VRULEX + HASHWIDTH / 2, (LONG) height - 1 );

    for (hunnert = VRULESTART;
	((hunnert * 100)) <= height; ++hunnert)
    {
	/* wide hashmark at X00	*/
	pos = hunnert * 100;
	endpos = pos + 100;
	Move( rp, VRULEX, (LONG) pos);
	Draw( rp,  VRULEX  + 2 * HASHWIDTH, (LONG) pos);

	while (  (pos += 10) < endpos )
	{
	    if ( pos > (height - 1) ) goto BREAK2;
	    Move( rp, VRULEX, (LONG) pos);
	    Draw( rp,  VRULEX  + HASHWIDTH, (LONG) pos);
	}

	sprintf( posbuff, "%d", endpos);
	tlength = TextLength( rp, posbuff, (LONG) strlen( posbuff ) );
	Move( rp, VRULEX - tlength - 1, (LONG) endpos - 1);
	Text( rp, posbuff, (LONG) strlen( posbuff ) );
    }
BREAK2:
    return;
}

struct Window	*
OpenWindowTags( nw,tags)
struct NewWindow	*nw;
ULONG			tags;
{
    struct Window	*OpenWindowTagList();

    return ( OpenWindowTagList( nw, &tags ) );
}
struct Screen	*
OpenScreenTags( ns, tag1, data1 )
struct NewScreen	*ns;
ULONG			tag1;
{
    struct Screen	*OpenScreenTagList();

    return ( OpenScreenTagList( ns, &tag1 ) );
}
