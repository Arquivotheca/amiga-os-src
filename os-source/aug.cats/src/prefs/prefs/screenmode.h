/*------------------------------------------------------------------------*/

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

#define min __builtin_min
#define max __builtin_max
extern int min (int, int);
extern int max (int, int);

/*  We get one of these for each available mode: */
struct DisplayMode
    {
    struct Node dm_Node;
    ULONG dm_ID;
    char dm_Name[DISPLAYNAMELEN];
    };

/*------------------------------------------------------------------------*/

/*  Absolute minimum size.  We decree that it shall be 640 x 200 for
    compatibility with old software, and as a reasonable minimum. */

#define ABSMINWIDTH	640
#define ABSMINHEIGHT	200

/*------------------------------------------------------------------------*/

#define TASK_NAME	"ScreenMode_Prefs"
#define ENV_NAME	"ENV:sys/screenmode.prefs"
#define ARC_NAME	"ENVARC:sys/screenmode.prefs"
#define PRESET_DIR	"Sys:Prefs/Presets"
#define PRESET_NAME	"screenmode.pre"

/*  non-valid string pointers for Initial and Default prefs settings */
#define INITIAL_PREFS	((STRPTR)-1)
#define DEFAULT_PREFS	(NULL)

/*  for full filename */
#define NAMEBUFSIZE	512

/*------------------------------------------------------------------------*/

/*  for ScanGadgets(): */
#define SCAN_WIDTH	1
#define SCAN_HEIGHT	2
#define SCAN_AUTOSCROLL	4
#define SCAN_ALL	~0

/*------------------------------------------------------------------------*/

#ifdef DEBUGGING
#define DP(x)	kprintf x
#define D(x)	x
void kprintf(char *,...);
#else
#define DP(x)	;
#define D(x)	;
#endif

/*------------------------------------------------------------------------*/

/* File errors */
#define ST_OK		(0) 
#define ST_BAD_OPEN	(1)
#define ST_BAD_READ	(2)
#define ST_BAD_WRITE	(3)

/*------------------------------------------------------------------------*/

/*  The contents of a Preferences for this editor: */
struct ScreenModePref
    {
    ULONG Reserved[4];
    ULONG id;
    UWORD width;
    UWORD height;
    UWORD depth;
    UWORD control;

    BOOL defwidth;
    BOOL defheight;
    UWORD index;
    struct DisplayMode *mode;
    };

/*  !!! NB:  This must match struct IWBPrefs in iprefs.h */
/*  The part written to disk: */
struct diskScreenModePref
    {
    ULONG Reserved[4];
    ULONG id;
    UWORD width;
    UWORD height;
    UWORD depth;
    UWORD control;
    };

/*------------------------------------------------------------------------*/
