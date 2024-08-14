/*******************************************************************/
/*        TO USE ONE OF THE FOLLOWING LIBRARIES, SET ITS FLAG TO 1 */
/*                            ELSE SET IT TO 0                     */
/*******************************************************************/

#define USE_EXEC 1
#define USE_DOS 1
#define USE_INTUITION 0
#define USE_GRAPHICS 1
#define USE_LAYERS 0
#define USE_ARP 0
#define USE_ICON 0
#define USE_DISKFONT 0
#define LIBVERSION 37

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
#include <dos/dos.h>
#include <dos/rdargs.h>
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
#include <graphics/displayinfo.h>
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
#include <libraries/diskFont.h>
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
	if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", LIBVERSION)) == NULL)
		Error ("Could not open the Intuition.library") ;
	#endif

	#if USE_GRAPHICS == 1
	/* Open the graphics library.... */
	if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", LIBVERSION)) == NULL)
		Error ("Could not open the Graphics.library") ;
	#endif

	#if USE_LAYERS == 1
	/* Open the layers library.... */
	if ((LayersBase = OpenLibrary ("layers.library", LIBVERSION)) == NULL)
		Error ("Could not open the layers.library") ;
	#endif

	#if USE_ARP == 1
	/* Open the arp library.... */
	if ((ArpBase = (struct ArpBase *)OpenLibrary ("arp.library", LIBVERSION)) == NULL)
		Error ("Could not open the arp.library") ;
	#endif

	#if USE_ICON == 1
	/* Open the icon library.... */
	if ((IconBase = OpenLibrary ("icon.library", LIBVERSION)) == NULL)
		Error ("Could not open the icon.library") ;
	#endif

	#if USE_DISKFONT == 1
	/* Open the diskfont library.... */
	if ((DiskfontBase = (struct DiskFontBase *)OpenLibrary ("diskfont.library", LIBVERSION)) == NULL)
		Error ("Could not open the DiskFont.library") ;
	#endif

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

void main (int argc, char *argv[])
{
	#define TEMPLATE "MUST/N,MUSTNOT/N,V/N,W/N,H/N,DW/N,DH/N,D/N,MID/N,SID/N,RBITS/N,GBITS/N,BBITS/N"
	#define OPT_MUST	0
	#define OPT_MUSTNOT	1
	#define OPT_VP		2
	#define OPT_W		3
	#define OPT_H		4
	#define OPT_DW		5
	#define OPT_DH		6
	#define OPT_D		7
	#define OPT_MID		8 
	#define OPT_SID		9
	#define OPT_RBITS	10
	#define OPT_GBITS	11
	#define OPT_BBITS	12
	#define OPT_COUNT	13

	ULONG ID;
	LONG result[OPT_COUNT];
	LONG *val;
	struct RDArgs *rdargs;
	struct TagItem ti[OPT_COUNT+1];
	struct TagItem *next = ti;
	int i;

	Init () ;

	for (i=0; i<OPT_COUNT;ti[i].ti_Tag = TAG_DONE, result[i] = NULL, i++);
	if (rdargs = ReadArgs(TEMPLATE, result, NULL))
	{
		if (val = (LONG *)result[OPT_MUST])
		{
			next->ti_Tag = BIDTAG_DIPFMustHave;
			next->ti_Data = *val;
			next++;
			printf("DIPFMustHave = 0x%lx\n", *val);
		}
		if (val = (LONG *)result[OPT_MUSTNOT])
		{
			next->ti_Tag = BIDTAG_DIPFMustNotHave;
			next->ti_Data = *val;
			next++;
			printf("DIPFMustNotHave = 0x%lx\n", *val);
		}
		if (val = (LONG *)result[OPT_VP])
		{
			next->ti_Tag = BIDTAG_ViewPort;
			next->ti_Data = *val;
			next++;
			printf("ViewPort = 0x%lx\n", *val);
		}
		if (val = (LONG *)result[OPT_W])
		{
			next->ti_Tag = BIDTAG_NominalWidth;
			next->ti_Data = *val;
			next++;
			printf("NominalWidth = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_H])
		{
			next->ti_Tag = BIDTAG_NominalHeight;
			next->ti_Data = *val;
			next++;
			printf("NominalHeight = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_DW])
		{
			next->ti_Tag = BIDTAG_DesiredWidth;
			next->ti_Data = *val;
			next++;
			printf("DesiredWidth = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_DH])
		{
			next->ti_Tag = BIDTAG_DesiredHeight;
			next->ti_Data = *val;
			next++;
			printf("DesredHeight = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_D])
		{
			next->ti_Tag = BIDTAG_Depth;
			next->ti_Data = *val;
			next++;
			printf("Depth = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_MID])
		{
			next->ti_Tag = BIDTAG_MonitorID;
			next->ti_Data = ((*val << 16) | 0x1000);
			next++;
			printf("MonitorID = 0x%lx\n", *val);
		}
		if (val = (LONG *)result[OPT_SID])
		{
			next->ti_Tag = BIDTAG_SourceID;
			next->ti_Data = *val;
			next++;
			printf("SourceID = 0x%lx\n", *val);
		}
		if (val = (LONG *)result[OPT_RBITS])
		{
			next->ti_Tag = BIDTAG_RedBits;
			next->ti_Data = *val;
			next++;
			printf("RBits = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_GBITS])
		{
			next->ti_Tag = BIDTAG_GreenBits;
			next->ti_Data = *val;
			next++;
			printf("GBits = %ld\n", *val);
		}
		if (val = (LONG *)result[OPT_BBITS])
		{
			next->ti_Tag = BIDTAG_BlueBits;
			next->ti_Data = *val;
			next++;
			printf("BBits = %ld\n", *val);
		}
		next->ti_Tag = TAG_DONE;
		ID = BestModeIDA(ti);
		printf("BestModeID = 0x%lx\n", ID);
		FreeArgs(rdargs);
	}
		
	CloseAll () ;
}
