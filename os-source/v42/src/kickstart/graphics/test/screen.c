/**********************************************************************/
/*                                                                    */
/*              Program : To open a screen of any mode and depth      */
/*                                                                    */
/*                      Written By : S.A.Shanson                      */
/*                     Date Started : Jun 12 1991                     */
/*              Last Modified : Wed Jun 12 23:38:11 1991              */
/*                            Language : C                            */
/*                                                                    */
/*  SourceFile : work:fun/screen/screen.c                             */
/*                                                                    */
/*  Called By : CLI/WB                                                */
/*                                                                    */
/**********************************************************************/

/*******************************************************************/
/*        TO USE ONE OF THE FOLLOWING LIBRARIES, SET ITS FLAG TO 1 */
/*                            ELSE SET IT TO 0                     */
/*******************************************************************/

#pragma libcall GfxBase SetRGB32 354 3210805
#pragma libcall GfxBase CalcIVG 33C 9802

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
#include <proto/exec.h>
#endif
#if USE_DOS == 1
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>
#endif
#if USE_INTUITION == 1
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/intuition.h>
#endif
#if USE_GRAPHICS == 1
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <proto/graphics.h>
#endif
#if USE_LAYERS == 1
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <proto/layers.h>
#endif
#if USE_ARP == 1
#include <arp/arpbase.h>
#include <arp/arpfunc.h>
#endif
#if USE_ICON == 1
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/icon.h>
#endif
#if USE_DISKFONT == 1
#include <libraries/diskfont.h>
#include <proto/diskfont.h>
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

void Init (ULONG modeid, UWORD depth)
{
	printf("depth = %ld\n", depth);
	#if USE_DOS == 1
	/* DOS library opened by Lattice */
	#endif

	#if USE_INTUITION == 1
	/* Open the intuition library.... */
	if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", 36L)) == NULL)
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
	if ((DiskFontBase = (struct DiskFontBase *)OpenLibrary ("diskfont.library", 0L)) == NULL)
		Error ("Could not open the DiskFont.library") ;
	#endif

	if ((s = OpenScreenTags(NULL,
					SA_Depth, depth,
					SA_DisplayID, modeid,
					TAG_DONE)) == NULL)
		Error("No screen\n");

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

	if (s)
		CloseScreen(s);

	#if USE_DISKFONT == 1
	/* Close the DiskFont library */
	if (DiskFontBase)
		CloseLibrary ((struct Library *) DiskFontBase) ;
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
		CloseLibrary (GfxBase) ;
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

void main (int argc, char *argv[])
{
	struct ViewPortExtra *vpe;
	ULONG modeid = 0x0;
	UWORD depth = 4;
	int i, j, p1;
	int x, y;
	SHORT width, height; 
	ULONG p;

	for (i = 1; i < argc; i++)
	{
		if (strncmp("0x", argv[i], 2) == 0)
		{
			modeid = strtoul(argv[i++], NULL, 0);
		}
		if (strcmp("d", argv[i]) == 0)
			depth = atoi(argv[++i]);
	}

	Init (modeid, depth) ;
	printf("fmode = %ld\n", CalcIVG(GfxBase->ActiView, &s->ViewPort));

	printf("depth = %lx, realdepth = %ld\n", depth, s->BitMap.Depth);

	switch (depth)
	{
		case 1 : x = 1; y = 2; break;
		case 2 : x = 2; y = 2; break;
		case 3 : x = 4; y = 2; break;
		case 4 : x = 4; y = 4; break;
		case 5 : x = 8; y = 4; break;
		case 6 : x = 8; y = 8; break;
		case 7 : x = 16; y = 8; break;
		case 8 : x = 16; y = 16; break;
		default : x = 0; y = 0; break;
	}

	width = s->Width / x;
	height = s->Height / y;

	p = 0;
	for (j = 0; j < (1 << depth); j++)
	{
		SetRGB32(&s->ViewPort, j, ((p & 0xff0000) << 8), ((p & 0xff00) << 16), ((p & 0xff) << 24));
		p += 0x123456;
	}

	s->RastPort.Mask = 0xff;

	RemakeDisplay();
	vpe = GfxLookUp((void *)&s->ViewPort);

	printf("vpe at 0x%lx\n", vpe);
	printf("vpe->Flags = 0x%x\n", vpe->Flags);
	printf("vpe->FMode = 0x%x\n", vpe->FMode);

	p1 = 0;
	for (j = 0; j < y; j++)
	{
		for (i = 0; i < x; i++)
		{
			SetAPen(&s->RastPort, p1++);
			RectFill(&s->RastPort, (i * width), (j * height), ((i * width) + width - 1), ((j * height) + height - 1));
		}
	}

	getchar();

	CloseAll();
}
