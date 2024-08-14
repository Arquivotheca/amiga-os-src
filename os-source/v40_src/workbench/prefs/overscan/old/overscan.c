/*** overscan.c ***********************************************************
*
*   Overscan.c	- Overscan Preferences Editor
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: overscan.c,v 36.11 91/01/15 12:17:10 eric Exp $
*
*   $Log:	overscan.c,v $
*   Revision 36.11  91/01/15  12:17:10  eric
*   Added WB and CLI argument support
*   
*   Revision 36.10  90/08/01  13:07:21  peter
*   The code which was conditional on "#define MODECHOICE" (not defined)
*   has been removed.  This code would have allowed the mode-within-monitor
*   selection that we used to have when Overscan had two scrolling lists.
*   Removed (compiled-out) code to handle absent-monitors, that is
*   monitors found in the .prefs file which aren't addmonitor'd.
*   
*   Revision 36.9  90/07/26  14:52:47  peter
*   Now include asl.h instead of aslbase.h.  AslBase is now struct Library *.
*   Now define _main as __stdargs to appease SAS/C 5.10.
*   
*   Revision 36.8  90/07/19  12:01:41  peter
*   The default height for a PAL machine was 512, not 256.  Now fixed.
*   
*   Revision 36.7  90/06/08  23:27:15  peter
*   Now establishes the four reserved longwords in the default prefs.
*   
*   Revision 36.6  90/06/01  21:39:24  peter
*   Now check DisplayInfo properties instead of GfxBase to sense a PAL
*   machine.
*   
*   Revision 36.5  90/05/15  23:44:00  peter
*   Defaults were still set for HIRESLACE size, though they were HIRES mode.
*   Changed current(monitor|name|display|dimension) to
*   db(monitor|name|display|dimension).
*   
*   Revision 36.4  90/05/15  00:38:07  peter
*   Conditionally compiled out code to handle editing of arbitrary modes.
*   Finished restoring use of default mode instead of explicit mode.
*   Changed DE_XXX labels to GAD_XXX labels.
*   Defaults to hires (not hires-lace).
*   
*   Revision 36.3  90/05/09  15:08:54  peter
*   Conditionally backed off use of explicit NTSC and PAL because of
*   difficulties.
*   
*   Revision 36.2  90/05/08  12:03:20  peter
*   Reset To Defaults on a PAL machine now adopts correct values.
*   Now always use explicit NTSC and PAL instead of default plus explicit other.
*   
*   Revision 36.1  90/04/19  23:43:46  peter
*   Now read prefs from ENV: upon startup, instead of using "current"
*   values from the database.
*   Not including <stdio.h> anymore.
*   
*   Revision 36.0  90/04/09  19:24:06  peter
*   Initial RCS check-in.
*   
*
**************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/displayinfo.h>
#include <graphics/monitor.h>
#include <graphics/gfxnodes.h>
#include <graphics/gfxbase.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <libraries/asl.h>

#include <string.h>

#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/icon_pragmas.h>

#include "overscan.h"
#include "display.h"

/*----------------------------------------------------------------------*/

#define TEMPLATE	"FROM,EDIT/S,USE/S,SAVE/S"
#define OPT_FROM	0
#define OPT_EDIT	1
#define OPT_USE		2
#define OPT_SAVE	3
#define OPT_COUNT	4

extern struct RDargs *ReadArgs(UBYTE *, LONG *, struct RDargs *);

extern struct WBStartup *WBenchMsg;

/* Action flags (determined from a project's ToolTypes) */
#define A_EDIT		0
#define A_USE		1
#define A_SAVE		2

/*----------------------------------------------------------------------*/

#define VERSION	36

struct List monitors;
struct Window *mywin;

ULONG globalNameIndex = 0;
ULONG globalIndex = 0;

struct MsgPort *DummyPort;

/*----------------------------------------------------------------------*/

/*  Functions defined elsewhere: */
struct Window *InitDisplay(void);
long HandleDisplayMessage(struct Window *win,
    struct IntuiMessage *imsg, BOOL inedit);
VOID FreeDisplay(struct Window *win);
void FreePrefsEntries(struct MinList *list);
BOOL BuildCurrentList(struct MinList *list);
BOOL BuildDefaultList(struct MinList *list);
WORD ReadOScanPrefs(char *name, BOOL Quiet);
WORD WriteOScanPrefs(char *name);
void AdoptPrefsEntries(struct MinList *list);

/*  Functions in this module: */

/* void __stdargs _main(void); */

VOID main(WORD argc, char *argv[]);
void bail_out(int code);
BOOL buildMonitorEntries(struct List *monitorlist);
BOOL readGfxDataBase(ULONG modeID, BOOL nocareavail);
BOOL AcceptableMode(ULONG modeID, struct DisplayInfo *qdisp,
    struct DimensionInfo *qdims, struct MonitorInfo *qmntr);
BOOL addMonitorEntry(ULONG modeID, struct List *monitorlist);

WORD rectWidth( struct Rectangle *rect );
WORD rectHeight( struct Rectangle *rect );

struct MsgPort *CreateEntryPort(char *name, BYTE pri);
void DeleteEntryPort(struct MsgPort *port);

/*------------------------------------------------------------------------*/

extern _exit (int code);

/*------------------------------------------------------------------------*/

extern struct Library *SysBase;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *UtilityBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *GadToolsBase = NULL;
struct Library *AslBase = NULL;
struct Library *IFFParseBase = NULL;
struct Library *IconBase = NULL;

LONG OldLock = 0;
BYTE *PresetName = 0;
BYTE action = A_EDIT;		/* Assume we are editing for now */
struct Remember *RKey = NULL;

struct EasyStruct edez0 =
{
    sizeof (struct EasyStruct), 0,
    "Overscan Preferences",
    "Could not read Overscan Preferences.\nResetting to defaults.",
    "OK",
};


struct MinList defaultlist;
struct MinList restorelist;

struct DisplayInfo   dbdisplay;
struct MonitorInfo   dbmonitor;
struct DimensionInfo dbdimension;

BOOL terminated;
BOOL machineispal = FALSE;


/*------------------------------------------------------------------------*/

/* void __stdargs _main(void) */
VOID main(argc, argv)
WORD argc;
char *argv[];
{
    LONG opts[OPT_COUNT];
    WORD status;
    struct RDargs *rdargs;
    BYTE **ttype, *tvalue;	/* ToolTypes array, value */
    BYTE unit = 0;		/* Unit number, if applicable */
    struct WBArg *arg;
    struct DiskObject *diskobj;
    struct IntuiMessage *imsg, localimsg;

    terminated = FALSE;

    NewList(&monitors);
    NewList((struct List *)&restorelist);

    /*  Open all libraries: */

    if (!(UtilityBase =
	OpenLibrary("utility.library", 36L)))
	bail_out(20);

    if (!(GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library", 37L)))
	bail_out(20);

    if (!(IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library", 36L)))
	bail_out(20);

    /* Only one of each editor at a time... */
    Forbid();
    if (DummyPort = FindPort(TASK_NAME))
    {
	/*  We've stuffed the window pointer here: */
	if (DummyPort->mp_SigTask)
	{
	    /*  Bring the existing one to front and activate it */
	    WindowToFront((struct Window *)DummyPort->mp_SigTask);
	    ActivateWindow((struct Window *)DummyPort->mp_SigTask);
	}
        Permit();
	DummyPort = NULL;
	bail_out(5);
    }
    /*  Priority -128 since we almost never search for it: */
    DummyPort = CreateEntryPort(TASK_NAME, (UBYTE)-128);
    Permit();
    if (! DummyPort )
    {
	bail_out(20);
    }

    if (!(GadToolsBase = OpenLibrary("gadtools.library", 36L)))
	bail_out(20);

    if (!(AslBase = OpenLibrary("asl.library", 14L)))
	bail_out(20);

    if (!(IFFParseBase = OpenLibrary("iffparse.library", 2L)))
	bail_out(20);

    if (!(IconBase = OpenLibrary("icon.library", 0L)))
	bail_out(20);

    if (argc == 0)				/* WB */
    {
	if (WBenchMsg->sm_NumArgs > 1)		/* WB: Preset */
	{
	    arg = WBenchMsg->sm_ArgList;
	    arg++;		/* skip editor name itself */
	    PresetName = (char *)arg->wa_Name;
	    OldLock = CurrentDir(arg->wa_Lock);

	    /* check for arguments */
	    if ((diskobj = GetDiskObject(arg->wa_Name)) != NULL)
	    {
		ttype = diskobj->do_ToolTypes;
		if ((tvalue = FindToolType(ttype, "ACTION")) != NULL)
		{
		    if (MatchToolValue(tvalue, "edit"))
			action = A_EDIT;
		    if (MatchToolValue(tvalue, "use"))
			action = A_USE;
		    if (MatchToolValue(tvalue, "save"))
			action = A_SAVE;
		}
		FreeDiskObject(diskobj);
	    }
/*	    CurrentDir(OldLock); */
	} 
    }
    else					/* CLI */
    {
	memset(opts, 0, sizeof(opts));
	if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
	{
	    if (opts[OPT_FROM])
	    {
		if ((PresetName = (BYTE *)AllocRemember(&RKey,
		  strlen((BYTE *)opts[OPT_FROM]) + 1, MEMF_CLEAR)) == NULL)
		{
		    FreeArgs(rdargs);
		    bail_out(20);
		}
		strcpy(PresetName, (BYTE *)opts[OPT_FROM]);
		D( kprintf("CLI FROM: %s\n", PresetName) );
	    }

	    /* Action */
	    if (opts[OPT_USE])
		action = A_USE;
	    if (opts[OPT_SAVE])
		action = A_SAVE;
	    if (opts[OPT_EDIT])
		action = A_EDIT;

	    FreeArgs(rdargs);
	}
    }


    readGfxDataBase( DEFAULT_MONITOR_ID, TRUE );
    if (dbdisplay.PropertyFlags & DIPF_IS_PAL)
    {
	machineispal = TRUE;
    }

    DP(("Welcome.  Calling buildMonitorEntries\n"));
    /*  Find all distinct monitors: */
    if ( !buildMonitorEntries(&monitors) )
    {
	bail_out(20);
    }

    NewList((struct List *)&defaultlist);
    if (!( BuildDefaultList(&defaultlist) ))
 	{
	bail_out(20);
	}

    if (PresetName)
    {
	if ((status = ReadOScanPrefs(PresetName, TRUE)) != ST_OK)
	{
	    D( kprintf("Reseting to defaults...\n") );
	    EasyRequest(NULL, &edez0, 0, 0);
	    /* ReadOScanPrefs(DEFAULT_PREFS, FALSE); */
	    AdoptPrefsEntries(&defaultlist);
	}
	/* PModified = TRUE; */
    }
    else
    {
	/* current = defaultpref; */
	if ((status = ReadOScanPrefs(ENV_NAME, TRUE)) != ST_OK)
	{
	    D(kprintf("Reseting to defaults...\n"));
	    if (status == ST_BAD_READ)
		EasyRequest(NULL, &edez0, 0, 0);
	    /* ReadOScanPrefs(DEFAULT_PREFS, FALSE); */
	    AdoptPrefsEntries(&defaultlist);
	}
    }

    /* initial = current; */
    if (!( BuildCurrentList(&restorelist) ))
	{
	bail_out(20);
	}

    if ((action == A_SAVE) || (action == A_USE))
    { 
	WriteOScanPrefs(ENV_NAME);
	if (action == A_SAVE)
	    WriteOScanPrefs(ARC_NAME);
	bail_out(0);
    }


    if (!(mywin = InitDisplay()))
	bail_out(20);

    while (!terminated)
    {
	Wait (1 << mywin->UserPort->mp_SigBit);
	while ((!terminated) && (imsg = GT_GetIMsg(mywin->UserPort)))
	{
	    localimsg = *imsg;
	    GT_ReplyIMsg(imsg);

	    terminated = HandleDisplayMessage(mywin, &localimsg, FALSE);
	}
    }
    /*  Close window before we Write prefs, to avoid any risk of an
	unneccessary requester from IPrefs */
    if (mywin)
    {
	FreeDisplay(mywin);
	mywin = NULL;
    }

    if (terminated == GAD_SAVE)
    {
	/*  submit to permanent prefs */
	WriteOScanPrefs(ARC_NAME);
    }

    if ((terminated == GAD_SAVE) || (terminated == GAD_USE))
    {
	/*  submit to volatile prefs */
	WriteOScanPrefs(ENV_NAME);
    }
    bail_out(0);
}


/*------------------------------------------------------------------------*/

/*/ bail_out()
 *
 * Close any allocated or opened stuff.
 *
 * Created:  30-May-89, Peter Cherna
 * Modified:  1-Dec-89, Peter Cherna
 *
 * Bugs: none
 *
 * Returns: does not return
 *
 */

void bail_out(code)
int code;
{
    /* Free lock from Wb args */
    if (OldLock)
	CurrentDir(OldLock);

    if (mywin)
    {
	DP(("Freeing display\n"));
	FreeDisplay(mywin);
    }

    DP(("Freeing restorelist\n"));
    FreePrefsEntries(&restorelist);

    DP(("Freeing MonitorEntries\n"));
    while (!Empty(&monitors))
    {
	struct MonitorEntry *mon;

	mon = (struct MonitorEntry *)RemTail( &monitors );
	FreeVec( mon );
    }
    DP(("Done Freeing\n"));

    if (RKey)
	FreeRemember(&RKey, TRUE);

    if (IconBase)
	CloseLibrary(IconBase);

    if (IFFParseBase)
	CloseLibrary(IFFParseBase);

    if (AslBase)
	CloseLibrary(AslBase);

    if (GadToolsBase)
	CloseLibrary(GadToolsBase);

    if (DummyPort)
    {
	DeleteEntryPort(DummyPort);
    }

    if (IntuitionBase)
	CloseLibrary(IntuitionBase);

    if (GfxBase)
	CloseLibrary((struct Library *)GfxBase);

    if (UtilityBase)
	CloseLibrary(UtilityBase);

    _exit(code);
}

/*----------------------------------------------------------------------*/

/* Scan the graphics database, looking for available modes.
 * If an available mode is found to be "acceptable", then
 * an entry is created for its monitor, if one doesn't yet exist.
 */
BOOL buildMonitorEntries( monitorlist )

struct List *monitorlist;

{
    ULONG modeID;

    /* start with special begin/end-of-list ID */
    modeID = INVALID_ID; 

    /* get next modeID until there are no more */

    while ((modeID = NextDisplayInfo( modeID )) != INVALID_ID)
    {
	/* We use explicit NTSC and PAL, instead of default
	 * plus explicit other
	 */
	if ( MONITOR_PART(modeID) )
	{
	    if ( readGfxDataBase( modeID, FALSE ) ) 
	    {
		/*  If the mode is available and supports a Workbench,
		    then we can consider it: */
		if (AcceptableMode(modeID, &dbdisplay, &dbdimension,
		    &dbmonitor))
		{
		    if ( !addMonitorEntry( modeID, monitorlist ) )
		    {
			/* Allocation failure */
			return(FALSE);
		    }
		}
	    }
	}
    }
    /* Signifies success */
    return(TRUE);
}


/*----------------------------------------------------------------------*/

/* Read the graphics database for the specified mode, and fill in
 * the global variables dbdisplay, dbmonitor, and dbdimension.
 */

BOOL readGfxDataBase( modeID, nocareavail )
ULONG modeID;
BOOL nocareavail;
{
    int info;
    int mntr = FALSE;
    int dims = FALSE;
    int success = FALSE;

    if(info = GetDisplayInfoData(NULL, (UBYTE *)&dbdisplay,
	sizeof(dbdisplay), DTAG_DISP, modeID))
    {
	mntr = GetDisplayInfoData(NULL, (UBYTE *)&dbmonitor,
	    sizeof(dbmonitor), DTAG_MNTR, modeID);
	dims = GetDisplayInfoData(NULL, (UBYTE *)&dbdimension,
	    sizeof(dbdimension), DTAG_DIMS, modeID);
	if ((!(dbdisplay.NotAvailable)) || nocareavail)
	    success = TRUE; 
    }
    return((BOOL) ((info && mntr && dims)? success : FALSE ));
}

/*----------------------------------------------------------------------*/

/*/ AcceptableMode()
 *
 * Given an ID and the DisplayInfo, DimensionInfo, and MonitorInfo
 * return TRUE if this is an acceptable mode for the job at hand,
 * else return FALSE.
 *
 * Created:  14-Sep-89, Peter Cherna
 * Modified: 16-Feb-90, Peter Cherna
 *
 */

BOOL AcceptableMode(id, qdisp, qdims, qmntr)

ULONG id;
struct DisplayInfo *qdisp;
struct DimensionInfo *qdims;
struct MonitorInfo *qmntr;

{
    BOOL ok;

    ok = TRUE;

    if (!(qdisp->PropertyFlags & DIPF_IS_WB))
	ok = FALSE;

    return(ok);
}

/*----------------------------------------------------------------------*/

/*/ addMonitorEntry()
 *
 * If the monitor of the specified mode is not yet on the supplied list,
 * make a MonitorEntry for it.
 */

BOOL addMonitorEntry( modeID, monitorlist )

ULONG modeID;
struct List *monitorlist;

{
    struct MonitorEntry *mon;

    DP(("addMonitorEntry( mode=%lx )\n", modeID ));
    for ( mon = (struct MonitorEntry *)monitorlist->lh_Head;
	mon->me_Node.ln_Succ;
	mon = (struct MonitorEntry *)mon->me_Node.ln_Succ)
    {
	DP(("Comparing against ID %lx\n", mon->me_ID));
	if ( MONITOR_PART( mon->me_ID ) == MONITOR_PART( modeID ) )
	{
	    DP(("aME(): We already have this monitor!\n"));
	    return( TRUE );
	}
    }

    if (!(mon = AllocVec( sizeof(struct MonitorEntry), MEMF_CLEAR )))
    {
	return( FALSE );
    }

    readGfxDataBase( modeID, FALSE );
    mon->me_ID = dbmonitor.PreferredModeID;
    readGfxDataBase( mon->me_ID, FALSE );

    DP(("Mode $%lx has aspect of (%ld,%ld)\n", mon->me_ID,
	dbdisplay.Resolution.x, dbdisplay.Resolution.y));
    /*  If the Monitor has a name, use that, else call it
	"Monitor N" */
    if ((dbmonitor.Mspc) && (dbmonitor.Mspc->ms_Node.xln_Name))
    {
	stccpy(mon->me_Name, dbmonitor.Mspc->ms_Node.xln_Name, MENAMELEN);
    }
    else
    {
	sprintf(mon->me_Name, "Monitor %ld", (LONG)(++globalNameIndex));
    }

    mon->me_Node.ln_Name = mon->me_Name;
    mon->me_Index = globalIndex++;
    mon->me_DisplayInfo = dbdisplay;
    mon->me_DimensionInfo = dbdimension;
    mon->me_MonitorInfo = dbmonitor;
    
    DP(("aME: monitor '%s', preferred mode $%lx\n", mon->me_Name,
	mon->me_ID));

    AddTail( monitorlist, (struct Node *)mon );
    return( TRUE );
}

/*----------------------------------------------------------------------*/

WORD rectWidth( rect )

struct Rectangle *rect;

{
    return( rect->MaxX - rect->MinX + 1 );
}

/*----------------------------------------------------------------------*/

WORD rectHeight( rect )

struct Rectangle *rect;

{
    return( rect->MaxY - rect->MinY + 1 );
}

/*----------------------------------------------------------------------*/

struct MsgPort *CreateEntryPort(name, pri)

char *name;
BYTE  pri;

{
    struct MsgPort *port;

    if (!(port = AllocMem(sizeof(struct MsgPort), MEMF_CLEAR | MEMF_PUBLIC)))
    {
	return (NULL);
    }

    port->mp_Node.ln_Name = name;
    port->mp_Node.ln_Pri = pri;
    port->mp_Node.ln_Type = NT_MSGPORT;

    port->mp_Flags = PA_IGNORE;

    if (name != 0)
	AddPort(port);

    return(port);
}

/*----------------------------------------------------------------------*/

void DeleteEntryPort(port)

struct MsgPort *port;

{
    if ((port->mp_Node.ln_Name))
    {
	RemPort(port);
    }
    FreeMem(port, sizeof(struct MsgPort));
}

/*----------------------------------------------------------------------*/
