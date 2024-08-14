/**********************************************************************/
/*                                                                    */
/*              Program : test scrollvp               */
/*                                                                    */
/*                      Written By : S.A.Shanson                      */
/*                     Date Started :                      */
/*              Last Modified :               */
/*                            Language : C                            */
/*                                                                    */
/*  SourceComp : Commodore Amiga                                      */
/*  SourceFile : */
/*                                                                    */
/*  Called By : CLI/WB                                                */
/*                                                                    */
/**********************************************************************/

/*******************************************************************/
/*        TO USE ONE OF THE FOLLOWING LIBRARIES, SET ITS FLAG TO 1 */
/*                            ELSE SET IT TO 0                     */
/*******************************************************************/

#define USE_EXEC 1
#define USE_DOS 1
#define USE_INTUITION 1
#define USE_GRAPHICS 1
#define USE_LAYERS 0
#define USE_ARP 0
#define USE_ICON 0
#define USE_DISKFONT 0

/*******************************************************************/
/*             TO DISABLE CTRL-C HANDLING, SET NO_CTRL_C TO 1      */
/*******************************************************************/

#define NO_CTRL_C 1

/*******************************************************************/
/*             TO DISABLE CBACK OPTION, SET NO_CBACK TO 1          */
/*******************************************************************/

#define NO_CBACK 1

#if NO_CTRL_C == 1
int CXBRK (void) { return (0) ; }	/* disable ctrl-c */
#endif

/*******************************************************************/
/*             TO ENABLE DEBUG MESSAGES, SET DEBUG TO 1            */
/*******************************************************************/

#define DEBUG 0

#include <exec/types.h>
#if USE_EXEC == 1
#include <exec/exec.h>
#include <clib/exec_protos.h>
#endif
#if USE_DOS == 1
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <clib/dos_protos.h>
#endif
#if USE_INTUITION == 1
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <clib/intuition_protos.h>
#endif
#if USE_GRAPHICS == 1
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <clib/graphics_protos.h>
#endif
#if USE_LAYERS == 1
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <clib/layers_protos.h>
#endif
#if USE_ARP == 1
#include <arp/arpbase.h>
#include <arp/arpfunc.h>
#endif
#if USE_ICON == 1
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <clib/icon_protos.h>
#endif
#if USE_DISKFONT == 1
#include <libraries/diskFont.h>
#include <clib/diskfont_protos.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/*********************************************************************/
/*                            GLOBAL VARIABLES                       */
/*********************************************************************/

#if USE_DOS == 1
/* DOSBase set up by Lattice */
/* struct DOSBase *DOSBase = NULL ; */
#endif
#if USE_INTUITION == 1
struct IntuitionBase *IntuitionBase = NULL ;
#endif
#if USE_GRAPHICS == 1
struct GfxBase *GfxBase = NULL ;
#endif
#if USE_LAYERS == 1
struct Library *LayersBase = NULL ;
#endif
#if USE_ARP == 1
struct ArpBase *ArpBase = NULL ;
#endif
#if USE_ICON == 1
struct Library *IconBase = NULL ;
#endif
#if USE_DISKFONT == 1
struct DiskFontBase *DiskfontBase = NULL ;
#endif

struct Screen *s = NULL;
struct Window *w = NULL;

struct NewWindow newwindow =
    {
        20,
        20,
        250,
        150,
        1, 2,
        VANILLAKEY|CLOSEWINDOW, /* Tell program when close gadget has been hit */
        WINDOWCLOSE | SMART_REFRESH | ACTIVATE | WINDOWDRAG |
        WINDOWDEPTH | WINDOWSIZING | NOCAREREFRESH,
        NULL,             /* Pointer to the first gadget -- */
                          /*   may be initialized later.    */
        NULL,             /* No checkmark.   */
        "ScrollVP() Test",  /* Window title.   */
        NULL,             /* Attach a screen later.  */
        NULL,             /* No bitmap.          */
        80,     /* Minimum width.      */
        19,    /* Minimum height.     */
        0xFFFF,           /* Maximum width.      */
        0xFFFF,           /* Maximum height.     */
        CUSTOMSCREEN      /* A screen of our own. */
    };

ULONG modeid = 0x0;
UWORD depth = 4;
UWORD uw = 320, h = 200;

/*******************************************************************/
/*                            CBack Variables                      */
/*******************************************************************/

#if NO_CBACK == 0
long _stack = STACKSIZE ;
char *_procname = PROCNAME ;
long _priority = PROCPRI ;
long _BackGroundIO = 1 ;		/* perform background IO */
extern BPTR _Backstdout ;		/* file handle */
#endif

/**********************************************************************/
/*                                                                    */
/* void Error (char *String)                                          */
/* Print string and exit                                              */
/*                                                                    */
/**********************************************************************/

void Error (char *String)
{
	void CloseAll (void) ;
	
	printf (String) ;

/*	if (_Backstdout) {
		Forbid () ;
		Write (_Backstdout, String, strlen (String)) ;
		Permit () ;
	}*/

	CloseAll () ;
	exit(0) ;
}


/**********************************************************************/
/*                                                                    */
/* void Init ()                                                       */
/*                                                                    */
/* Opens all the required libraries                                   */
/* allocates all memory, etc.                                         */
/*                                                                    */
/**********************************************************************/

void Init ()
{
	#if USE_DOS == 1
	/* DOS library opened by Lattice */
	#endif

	#if USE_INTUITION == 1
	/* Open the intuition library.... */
	if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", 0L)) == NULL)
		Error ("Could not open the Intuition.library") ;
	#endif

	#if USE_GRAPHICS == 1
	/* Open the graphics library.... */
	if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", 0L)) == NULL)
		Error ("Could not open the Graphics.library") ;
	#endif

	#if USE_LAYERS == 1
	/* Open the layers library.... */
	if ((LayersBase = OpenLibrary ("layers.library", 0L)) == NULL)
		Error ("Could not open the layers.library") ;
	#endif

	#if USE_ARP == 1
	/* Open the arp library.... */
	if ((ArpBase = (struct ArpBase *)OpenLibrary ("arp.library", 0L)) == NULL)
		Error ("Could not open the arp.library") ;
	#endif

	#if USE_ICON == 1
	/* Open the icon library.... */
	if ((IconBase = OpenLibrary ("icon.library", 0L)) == NULL)
		Error ("Could not open the icon.library") ;
	#endif

	#if USE_DISKFONT == 1
	/* Open the diskfont library.... */
	if ((DiskfontBase = (struct DiskFontBase *)OpenLibrary ("diskfont.library", 0L)) == NULL)
		Error ("Could not open the DiskFont.library") ;
	#endif

	if ((s = OpenScreenTags(NULL, 
				SA_Width, uw, SA_Height, h,
				SA_Depth, depth,
				SA_DisplayID, modeid,
				SA_Overscan, OSCAN_TEXT,
				TAG_DONE)) == NULL)
		Error("No screen");

	newwindow.Screen = s;
	if ((w = OpenWindow(&newwindow)) == NULL)
		Error("No window");

	return ;
}

/**********************************************************************/
/*                                                                    */
/* void CloseAll ()                                                   */
/*                                                                    */
/* Closes and tidies up everything that was used.                     */
/*                                                                    */
/**********************************************************************/

void CloseAll ()
{
	/* Close everything in the reverse order in which they were opened */

	if (w)
		CloseWindow(w);

	if (s)
		CloseScreen(s);

	#if USE_DISKFONT == 1
	/* Close the DiskFont library */
	if (DiskFontBase)
		CloseLibrary ((struct Library *) DiskfontBase) ;
	#endif

	#if USE_ICON == 1
	/* Close the icon library */
	if (IconBase)
		CloseLibrary (IconBase) ;
	#endif

	#if USE_ARP == 1
	/* Close the arp library */
	if (ArpBase)
		CloseLibrary ((struct Library *) ArpBase) ;
	#endif

	#if USE_LAYERS == 1
	/* Close the layers Library */
	if (LayersBase)
		CloseLibrary (LayersBase) ;
	#endif

	#if USE_GRAPHICS == 1
	/* Close the Graphics Library */
	if (GfxBase)
		CloseLibrary ((struct Library *) GfxBase) ;
	#endif

	#if USE_INTUITION == 1
	/* Close the Intuition Library */
	if (IntuitionBase)
		CloseLibrary ((struct Library *) IntuitionBase) ;
	#endif

	#if USE_DOS == 1
	/* DOS closed by Lattice */
	#endif

	return ;
}

/***************************************************************************/

VOID movePF( WORD dx, WORD dy, struct Window *win)
{
	struct RasInfo *rinfo2 = win->WScreen->ViewPort.RasInfo;

    rinfo2->RxOffset += dx;
    if (rinfo2->RxOffset < 0)
	rinfo2->RxOffset = 99;
    else if (rinfo2->RxOffset > 99)
	rinfo2->RxOffset = 0;

    rinfo2->RyOffset += dy;
    if (rinfo2->RyOffset < 0)
	rinfo2->RyOffset = 99;
    else if (rinfo2->RyOffset > 99)
	rinfo2->RyOffset = 0;

	ScrollVPort(&win->WScreen->ViewPort);
}

UBYTE handleIDCMP( struct Window *win )
{
    UBYTE flag = 0;
    struct IntuiMessage *message = NULL;
    ULONG class;
    UWORD code;
	int j;

    while( message = (struct IntuiMessage *)GetMsg(win->UserPort) ) {

        class = message->Class;
        code = message->Code;
        ReplyMsg( (struct Message *)message);

        switch( class ) {
        
            case CLOSEWINDOW:
                flag = 1;
                break;
                
	    case VANILLAKEY:
		switch( code ) {

		    case 'u':
		    case 'U':
				movePF(0,1,win);
		        break;

		    case 'd':
		    case 'D':
		        movePF(0,-1,win);
		        break;

		    case 'l':
		    case 'L':
		        movePF(1,0,win);
		        break;

		    case 'r':
		    case 'R':
		        movePF(-1,0,win);
		        break;
		}
		
            default:
                break;
                
        }        
    }

    return(flag);
}
void main (int argc, char *argv[])
{
	UBYTE done = 0;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strncmp("0x", argv[i], 2) == 0)
		{
			modeid = strtoul(argv[i++], NULL, 0);
		}
		if (strcmp("d", argv[i]) == 0)
			depth = atoi(argv[++i]);
		if (strcmp("w", argv[i]) == 0)
			uw = atoi(argv[++i]);
		if (strcmp("h", argv[i]) == 0)
			h = atoi(argv[++i]);
	}

	Init () ;

	while( !done ) {
		WaitPort(w->UserPort);
		done = handleIDCMP(w);
	};

	CloseAll () ;
}
