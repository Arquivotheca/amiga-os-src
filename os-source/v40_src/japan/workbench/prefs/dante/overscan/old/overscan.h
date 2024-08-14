#define RELEASE

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif !GRAPHICS_DISPLAYINFO_H

/*------------------------------------------------------------------------*/

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/asl_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/asl_pragmas.h>

/*------------------------------------------------------------------------*/

extern struct Library *SysBase;
extern struct DosLibrary *DOSBase;
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library *GadToolsBase;
extern struct Library *IFFParseBase;
extern struct Library *AslBase;

/*------------------------------------------------------------------------*/

VOID __stdargs sprintf(STRPTR s, STRPTR fmt, ...);

/*------------------------------------------------------------------------*/

#define Empty(list) (!((list)->lh_Head->ln_Succ))
#define HeadNode(list)	((list)->lh_Head)
#define TailNode(list)	((list)->lh_TailPred)

#define MENAMELEN 30

/* Every monitor we find will get a MonitorEntry structure */
struct MonitorEntry
    {
    struct Node me_Node;			/* For linking */
    WORD me_Index;
    ULONG me_ID;				/* ModeID to use for edits */
    struct DisplayInfo me_DisplayInfo;		/* data for that mode */
    struct DimensionInfo me_DimensionInfo;
    struct MonitorInfo me_MonitorInfo;
    char me_Name[MENAMELEN];			/* Monitor name */
    };

/*  Some stuff is saved as it would be in the Prefs file: */

struct PrefsEntry
    {
    struct MinNode Node;
    ULONG Reserved[4];
    ULONG ID;
    Point ViewPos;
    Point Txt;
    struct Rectangle Std;
    };

struct diskPrefsEntry
    {
    ULONG Reserved[4];
    ULONG ID;
    Point ViewPos;
    Point Txt;
    struct Rectangle Std;
    };


#define MONITOR_PART(id)	((id) & MONITOR_ID_MASK)

/*  Codes for variable oscantype, tells us which rectangle user is
    working on, or if he just wants to inspect them: */
#define JUSTLOOKING 0
#define TXTOSCAN    1
#define STDOSCAN    2

#define TASK_NAME	"Overscan_Prefs"
#define ENV_NAME	"ENV:sys/overscan.prefs"
#define ARC_NAME	"ENVARC:sys/overscan.prefs"
#define PRESET_DIR	"Sys:Prefs/Presets"
#define PRESET_NAME	"Overscan.pre"

/*  for full filename */
#define NAMEBUFSIZE	512

void kprintf(char *,...);
#ifdef DEBUGGING
#define DP(x)	kprintf x
#define D(x)	x
#define DRECT(str,rect)	kprintf(str);kprintf(": [%ld,%ld]-[%ld,%ld]\n",	\
	rect.MinX,rect.MinY,rect.MaxX,rect.MaxY);
#else
#define DP(x)	;
#define D(x)	;
#define DRECT(str,rect)	;
#endif

#ifdef MAXDEBUG
#define MP(x)	kprintf x
#define M(x)	x
#else
#define MP(x)	;
#define M(x)	;
#endif

/*------------------------------------------------------------------------*/

/* File errors */
#define ST_OK		(0) 
#define ST_BAD_OPEN	(1)
#define ST_BAD_READ	(2)
#define ST_BAD_WRITE	(3)

/*------------------------------------------------------------------------*/

#define RED_MASK	0x0F00
#define GREEN_MASK	0x00F0
#define BLUE_MASK	0x000F

#define RED_SHIFT	8
#define GREEN_SHIFT	4
#define BLUE_SHIFT	0

