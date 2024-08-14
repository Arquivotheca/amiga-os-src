/**************************************************************************
*
*   diskio.c	-  I/O routines for Overscan Editor
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: diskio.c,v 36.6 91/01/15 12:09:06 eric Exp $
*
*   $Log:	diskio.c,v $
*   Revision 36.6  91/01/15  12:09:06  eric
*   Release 2.03
*   
*   Revision 36.5  90/08/01  13:07:44  peter
*   The code which was conditional on "#define MODECHOICE" (not defined)
*   has been removed.  This code would have allowed the mode-within-monitor
*   selection that we used to have when Overscan had two scrolling lists.
*   Removed (compiled-out) code to handle absent-monitors, that is
*   monitors found in the .prefs file which aren't addmonitor'd.
*   I've enabled the MapDefault() code so that overscan.prefs files can be
*   safely moved between NTSC and PAL machines, will remaining compatible
*   with Release 2.00 files.
*   
*   Revision 36.4  90/06/08  23:26:45  peter
*   Now uses OSCN chunk ID instead of PDAT.
*   Now corrects for the four reserved longwords.
*   
*   Revision 36.3  90/06/01  21:38:53  peter
*   Now can write explicit mode instead of default, and convert it back
*   to default at read-time (Commented out).
*   
*   Revision 36.2  90/05/15  23:43:22  peter
*   Changed current(monitor|name|display|dimension) to
*   db(monitor|name|display|dimension).
*   Quantizes data into view-resolution units as it is read in.
*   
*   Revision 36.1  90/05/15  00:33:07  peter
*   No longer depends on mg_ModeList to find a sample mode in a group, but
*   uses EditMode instead (saves space, helps eliminate second scrolling list).
*   Conditionally compiled out code to handle editing of arbitrary modes.
*   
*   Revision 36.0  90/04/09  19:22:43  peter
*   Initial RCS check-in.
*   
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/gadtools.h>

#include <libraries/iffparse.h>

#include <string.h>

#include "overscan.h"
#include "display.h"

/*------------------------------------------------------------------------*/

struct IFFHandle *AllocIFF();
struct ContextNode *CurrentChunk(struct IFFHandle *);

/*------------------------------------------------------------------------*/

BOOL readGfxDataBase(ULONG ID, BOOL nocareavail);
void ShiftRect(struct Rectangle *rect, WORD dx, WORD dy);
WORD modulo(WORD num, WORD div);
WORD rectWidth( struct Rectangle *rect );
WORD rectHeight( struct Rectangle *rect );

/*------------------------------------------------------------------------*/

WORD ReadOScanPrefs(STRPTR name, BOOL Quiet);
WORD WriteOScanPrefs(STRPTR name);

void AdoptPrefsEntries(struct MinList *list);
BOOL AddPrefsEntry(struct MinList *list, ULONG ID,
    WORD ViewPosX, WORD ViewPosY, WORD TxtX, WORD TxtY, struct Rectangle *Std);
BOOL BuildCurrentList(struct MinList *list);
BOOL BuildDefaultList(struct MinList *list);
void FreePrefsEntries(struct MinList *list);

WORD ReadData(STRPTR name, struct MinList *list, BOOL Quiet);
WORD WriteData(STRPTR name, struct MinList *list);
struct IFFHandle *GetIFF(BYTE *file, LONG mode);
void ReturnIFF(struct IFFHandle *iff);
LONG ErrorIFF(LONG error);
void MapDefaultMonitor(struct MinList *list, ULONG fromid, ULONG toid);
void FileError(WORD error, UBYTE *str1);

/*------------------------------------------------------------------------*/

extern struct List monitors;
extern struct DisplayInfo   dbdisplay;
extern struct MonitorInfo   dbmonitor;
extern struct DimensionInfo dbdimension;
extern BOOL machineispal;

struct MinList iolist;

/*------------------------------------------------------------------------*/

struct EasyStruct file_ez = { 
    sizeof (struct EasyStruct), 0,
    "Overscan Preferences",
    NULL,
    "OK",
};


#define ID_PREF		MAKE_ID('P','R','E','F')
#define ID_PRHD		MAKE_ID('P','R','H','D')
#define ID_OSCN		MAKE_ID('O','S','C','N')

struct PrefHeader {
    UBYTE ph_Version;
    UBYTE ph_Type;
    ULONG ph_test;
};

struct PrefHeader phead = {0, 0, 0L};

static LONG stops[] = {
    ID_PREF, ID_PRHD,
    ID_PREF, ID_OSCN
};

/*------------------------------------------------------------------------*/

/*/ ReadOScanPrefs()
 *
 * Reads IFF format Overscan prefs from the named file.
 * Returns TRUE if successful.
 *
 * Created:  11-Jan-90, Peter Cherna
 * Modified: 30-Jan-90, Peter Cherna
 */

WORD ReadOScanPrefs(name, Quiet)
STRPTR name;
BOOL Quiet;
{
    WORD retval = ST_OK;

    NewList((struct List *)&iolist);
    DP(("Trying to read '%s'\n", name));

    if ((retval = ReadData(name, &iolist, Quiet)) == ST_OK)
    {
	DP(("Successful\n"));
	/* Since we now deal in explicit NTSC and PAL, but Release 2.00
	 * Overscan prefs wrote out "default" modes, we have to map
	 * them into the appropriate choice.
	 */
	MapDefaultMonitor(&iolist, DEFAULT_MONITOR_ID, machineispal ?
	  PAL_MONITOR_ID : NTSC_MONITOR_ID);
	AdoptPrefsEntries(&iolist);
    }
    FreePrefsEntries(&iolist);

    return(retval);
}


/*------------------------------------------------------------------------*/

/*/ WriteOScanPrefs()
 *
 * Writes Overscan Prefs in IFF format.
 *
 * Created:  18-Sep-89, Peter Cherna
 * Modified: 30-Jan-90, Peter Cherna
 */

WORD WriteOScanPrefs(name)
STRPTR name;
{
    WORD retval = ST_BAD_OPEN;

    NewList((struct List *)&iolist);
    if (BuildCurrentList(&iolist))
    {
	if ((retval = WriteData(name, &iolist)) != ST_OK)
	{
	    DP(("Failed to Write %s\n", name));
	    DeleteFile(name);
	}
    }
    FreePrefsEntries(&iolist);

    return(retval);
}


/*------------------------------------------------------------------------*/

/*/ ReadData()
 *
 * Read IFF data from the supplied file name, adding to the supplied list
 * using AddPrefsEntry().
 *
 * Created:  Eric Cotton
 * Modified:  6-Apr-90, Peter Cherna
 *
 */

WORD ReadData(name, list, Quiet)
STRPTR name;
struct MinList *list;
BOOL Quiet;
{
    struct IFFHandle *iff;
    struct ContextNode *cn;
    BOOL status = ST_BAD_READ;
    BOOL HeaderFlag = FALSE;
    BOOL DataFlag = FALSE;
    LONG error;
    static struct diskPrefsEntry buffer;

    /* Allocate & initialize IFFHandle structure */
    if (!(iff = GetIFF(name, MODE_OLDFILE)))
    {
	DP(("RD:  GetIFF failed\n"));
	status = ST_BAD_OPEN;
	goto RD_Exit;
    }

    /* Initial parse */
    error = ParseIFF(iff, IFFPARSE_STEP);
    if (error)
    {
	if (error == IFFERR_EOF)
	{
	    DP(("No top chunk!\n"));
	}
	else
	{
	    ErrorIFF(error);
	}
	goto RD_Exit;
    }

    /* Make sure we have a pref file */
    cn = CurrentChunk(iff);
    if ((cn->cn_ID != ID_FORM) || (cn->cn_Type != ID_PREF))
    {
	DP(("Not a FORM PREF!\n"));
	goto RD_Exit;
    }

    /* Setup Stop Chunks: PRHD and PRDAT */
    if (error = StopChunks(iff, stops, 2L))
    {
	ErrorIFF(error);
	goto RD_Exit;
    }

    /* Loop until EOF (*usually* good) or error condition (bad) */
    while (TRUE)
    {
	/* Scan file for appropriate chunks */
	if (error = ParseIFF(iff, IFFPARSE_SCAN))
	{
	    if (error == IFFERR_EOF)
	    {
		if (HeaderFlag && DataFlag)
		    status = ST_OK;
		else
		{
		    DP(("File is incomplete!\n"));
		}
	    }
	    else
	    {
		ErrorIFF(error);
	    }
	    break;
	}

	/* Get the current context */
	cn = CurrentChunk(iff);

	/* Read a header chunk */
	if ((cn->cn_ID == ID_PRHD) && (cn->cn_Type == ID_PREF))
	{
	    error = ReadChunkBytes(iff, (void *)&phead, sizeof(struct PrefHeader));
	    if (error == sizeof(struct PrefHeader))
		HeaderFlag = TRUE;
	    else
	    {
		if (error < 0L)
		{
		    ErrorIFF(error);
		}
		else
		{
		    DP(("Short header!\n"));
		}
		break;
	    }
	}

	/* Read a data chunk */
	if ((cn->cn_ID == ID_OSCN) && (cn->cn_Type == ID_PREF))
	{
	    error = ReadChunkBytes(iff, (void *)&buffer, sizeof(struct diskPrefsEntry));
	    if (error == sizeof(struct diskPrefsEntry))
	    {
		/*  !!! Note no error will be generated if some entries
		    got added and some failed */
		DP(("Got OSCN chunk for ID $%lx\n", buffer.ID));
		if (AddPrefsEntry(list, buffer.ID, buffer.ViewPos.x,
		    buffer.ViewPos.y, buffer.Txt.x, buffer.Txt.y,
		    &buffer.Std))
		{
		    DataFlag = TRUE;
		}
	    }
	    else
	    {
		if (error < 0L)
		{
		    ErrorIFF(error);
		}
		else
		{
		    DP(("Short data!\n"));
		}
		break;
	    }
	}
    }

RD_Exit:
    D(kprintf("status = %ld\n", status));
    ReturnIFF(iff);
    if ((!Quiet) && (status != ST_OK))
	FileError(status, name);
    return(status);
}


/*------------------------------------------------------------------------*/

/*/ WriteData()
 *
 * Write IFF date from buffer to file of given name.
 *
 * Created:  Eric Cotton
 * Modified: 30-Jan-90, Peter Cherna
 *
 */

WORD WriteData(name, list)
STRPTR name;
struct MinList *list;
{
    struct IFFHandle *iff;
    BOOL status = ST_BAD_WRITE;
    LONG ierror = 0;
    static struct diskPrefsEntry *bufferptr;
    struct PrefsEntry *pentry;

    /* Allocate and initialize IFFHandle structure */
    if (!(iff = GetIFF(name, MODE_NEWFILE)))
    {
	DP(("WD: Failed to GetIFF\n"));
	status = ST_BAD_OPEN;
	goto WT_Exit;
    }

    /* Start the file */
    if (ierror = PushChunk(iff, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN))
    {
	DP(("WD:  Failed to PushChunk PREF\n"));
	goto WT_Exit;
    }

    /* Write the PrefHeader chunk */
    if (ierror = PushChunk(iff, 0, ID_PRHD, sizeof(struct PrefHeader)))
    {
	DP(("WD:  Failed to PushChunk PRHD\n"));
	goto WT_Exit;
    }
    if ((WriteChunkBytes(iff, (void *)&phead, sizeof(struct PrefHeader))) !=
      sizeof(struct PrefHeader))
    {
	DP(("WD:  Failed to WriteChunkBytes PRHD\n"));
	goto WT_Exit;
    }
    if (ierror = PopChunk(iff))
    {
	DP(("HD:  Failed to PopChunk PRHD\n"));
	goto WT_Exit;
    }

    /*  Write all the modes from the specified list: */
    for (pentry = (struct PrefsEntry *)list->mlh_Head;
	(pentry->Node.mln_Succ);
	pentry = (struct PrefsEntry *)pentry->Node.mln_Succ)
    {
	DP(("Writing chunk for id $%lx\n", pentry->ID));
	/*  Point to the part that we want to write to disk: */
	bufferptr = (struct diskPrefsEntry *)pentry->Reserved;
	if (ierror = PushChunk(iff, 0, ID_OSCN, IFFSIZE_UNKNOWN))
	{
	    DP(("WD:  Failed to PushChunk OSCN\n"));
	    goto WT_Exit;
	}
	if ((WriteChunkBytes(iff, (void *)bufferptr, sizeof(struct
	  diskPrefsEntry))) != sizeof(struct diskPrefsEntry))
	{
	    DP(("WD:  Failed to WriteChunkBytes OSCN\n"));
	    goto WT_Exit;
	}
	if (ierror = PopChunk(iff))
	{
	    DP(("HD:  Failed to PopChunk SCRM\n"));
	    goto WT_Exit;
	}
    }

    if (ierror = PopChunk(iff))
    {
	DP(("HD:  Failed to PopChunk PREF\n"));
	goto WT_Exit;
    }

    status = ST_OK;

WT_Exit:
    D(kprintf("status = %ld\n", status));
    if (ierror)
    {
	ErrorIFF(ierror);
    }
    ReturnIFF(iff);
    if (status != ST_OK)
	FileError(status, name);
    return (status);
}


/*------------------------------------------------------------------------*/

/*/ GetIFF()
 *
 * Prepare the named file for IFF reading.
 * Allocate IFF_File structure, open the named file, initialize it as
 * DOS, and OpenIFF().
 *
 * Created:	Eric Cotton
 * Modified:  30-Jan-90, Peter Cherna
 *
 */

struct IFFHandle *GetIFF(file, mode)
BYTE *file;
LONG mode;
{
    struct IFFHandle *iff;

    /* Allocate and IFFHandle structure */
    if (!(iff = AllocIFF()))
    {
	DP(("GI:  Failed to AllocIFF\n"));
        return(NULL);
    }

    /* Open a DOS stream */
    if (!(iff->iff_Stream = (ULONG)Open(file, mode)))
    {
	DP(("GI:  Failed to Open\n"));
	FreeIFF(iff);
	return(NULL);
    }

    InitIFFasDOS(iff);			/* Initialize iff as DOS */

    OpenIFF(iff, (mode == MODE_OLDFILE)? IFFF_READ : IFFF_WRITE);

    return(iff);
}


/*------------------------------------------------------------------------*/

/*/ ReturnIFF()
 *
 * Shut down and free an IFF_File structure started with GetIFF().
 *
 * Created:  Eric Cotton
 * Modified: 30-Jan-90, Peter Cherna
 *
 */

void ReturnIFF(iff)
struct IFFHandle *iff;
{
    if (iff)
    {
	if(iff->iff_Stream)
	{
	    CloseIFF(iff);
	    Close(iff->iff_Stream);
	}
	FreeIFF(iff);
    }
}


/*------------------------------------------------------------------------*/

/*/ ErrorIFF()
 *
 * Debug-Print an informative error message if there was trouble with
 * the IFF file.
 *
 * Created:  Eric Cotton
 * Modified: 30-Jan-90, Peter Cherna
 *
 */

LONG ErrorIFF(error)

    LONG error;

    {
    /* English error messages for possible IFFERR_#? returns from various
     * IFF routines.  To get the index into this array, take your IFFERR
     * code, negate it, and subtract one.
     *  idx = -error - 1;
     */
#ifdef DEBUGGING
    static char *errormsgs[] =
	{
	"End of file (not an error).",
	"End of context (not an error).",
	"No lexical scope.",
	"Insufficient memory.",
	"Stream read error.",
	"Stream write error.",
	"Stream seek error.",
	"File is corrupt.",
	"IFF syntax error.",
	"Not an IFF file.",
	"Return to client.  You should never see this."
	};

    if (error < 0)
	{
	DP(("IFF error: %s\n", errormsgs[(-error) - 1]));
	}
#endif
    return(error);
    }


/* ======================================================================== *
 * ==									 == *
 * ==	void FileError(WORD, UBYTE *)					 == *
 * ==									 == *
 * ======================================================================== */

void FileError(error, str1)
WORD error;
UBYTE *str1;
{
    if (error == ST_BAD_OPEN)
	file_ez.es_TextFormat = "Could not find\n%s";
    else if (error == ST_BAD_READ)
	file_ez.es_TextFormat = "Could not read\n%s";
    else if (error == ST_BAD_WRITE)
	file_ez.es_TextFormat = "Could not write\n%s";

    EasyRequestArgs(NULL, &file_ez, 0, &str1);
}


/*------------------------------------------------------------------------*/

/*/ AdoptPrefsEntries()
 *
 * Take a list of PrefsEntry's and apply them to my copies of 
 * the database info.
 *
 * Created:  11-Jan-90, Peter Cherna
 * Modified: 30-Jan-90, Peter Cherna
 *
 */

void AdoptPrefsEntries(struct MinList *list)
{
    BOOL found;
    struct MonitorEntry *mon;
    Point shift;
    struct PrefsEntry *pentry;
    struct MonitorInfo *moni;
    struct DisplayInfo *disi;
    struct DimensionInfo *dimi;
    UWORD xscale, yscale;

    for (pentry = (struct PrefsEntry *)list->mlh_Head;
	pentry->Node.mln_Succ;
	pentry = (struct PrefsEntry *)pentry->Node.mln_Succ)
    {
	DP(("Trying to adopt entry for mode $%lx\n", pentry->ID));
	/*  Find MonitorEntry for this entry.  If there's no such monitor,
	    we ignore.  If there is, we transmogrify the monitor's data. */
	found = FALSE;
	mon = (struct MonitorEntry *) monitors.lh_Head;
	while ((!found) && (mon->me_Node.ln_Succ))
	{
	    /*  is this the monitor? */
	    found = (mon->me_ID == pentry->ID);
	    if (!found)
	    {
		mon = (struct MonitorEntry *)mon->me_Node.ln_Succ;
	    }
	    D(else
	    {
		DP(("Found monitor including mode $%lx\n",
		    mon->me_ID));
	    });
	}

	/*  If we found the monitor, try to get the mode data, not caring
	    if the mode is available or not: */
	if ((found) && (readGfxDataBase(pentry->ID, TRUE)))
	{
	    /*  Shorthand saves eyestrain and 4 bytes: */
	    dimi = &mon->me_DimensionInfo;
	    disi = &mon->me_DisplayInfo;
	    moni = &mon->me_MonitorInfo;

	    *dimi = dbdimension;
	    *disi = dbdisplay;
	    *moni = dbmonitor;

	    /*  Correct overscan numbers to a multiple of view
		resolution, in case some files predate this constraint: */
	    xscale = moni->ViewResolution.x / disi->Resolution.x;
	    yscale = moni->ViewResolution.y / disi->Resolution.y;

	    /*  To calculate the MaxOScan rectangle, we take
		the database result, and consider that the ViewPos has
		shifted to pentry->ViewPos, and see the result on that
		rectangle: */

	    /*  Measure how much the ViewPosition shifted,
		in mode resolution: */
	    shift.x = ((moni->ViewPosition.x - pentry->ViewPos.x) *
		moni->ViewResolution.x) / disi->Resolution.x;
	    shift.y = ((moni->ViewPosition.y - pentry->ViewPos.y) *
		moni->ViewResolution.y) / disi->Resolution.y;
	
	    /*  Shift MaxOScan accordingly: */
	    ShiftRect(&dimi->MaxOScan, shift.x, shift.y);

	    /*  Install the TxtOScan and StdOScan we read in: */
	    dimi->TxtOScan.MinX = 0;
	    dimi->TxtOScan.MinY = 0;
	    dimi->TxtOScan.MaxX = pentry->Txt.x - modulo(pentry->Txt.x, xscale) - 1;
	    dimi->TxtOScan.MaxY = pentry->Txt.y - modulo(pentry->Txt.y, yscale) - 1;

	    dimi->StdOScan = pentry->Std;
	    DP(("Before rounding:\n"));
	    DP(("TxtOScan: [%ld,%ld]-[%ld,%ld]\n",
		(LONG)0,
		(LONG)0,
		(LONG)pentry->Txt.x - 1,
		(LONG)pentry->Txt.y - 1));
	    DRECT("StdOScan", dimi->StdOScan);
	    dimi->StdOScan.MinX -= modulo(dimi->StdOScan.MinX, xscale);
	    dimi->StdOScan.MinY -= modulo(dimi->StdOScan.MinY, yscale);
	    dimi->StdOScan.MaxX -= modulo((WORD)(dimi->StdOScan.MaxX+1), xscale);
	    dimi->StdOScan.MaxY -= modulo((WORD)(dimi->StdOScan.MaxY+1), yscale);

	    /*  Install the ViewPos we read in: */
	    moni->ViewPosition = pentry->ViewPos;

	    DP(("After shifting and sliding:\n"));
	    DRECT("TxtOScan", dimi->TxtOScan);
	    DRECT("StdOScan", dimi->StdOScan);
	    DRECT("MaxOScan", dimi->MaxOScan);
	}
    }
}

/*------------------------------------------------------------------------*/

/*/ AddPrefsEntry()
 *
 * Adds an entry (as it would be in a file) to a list for later use.
 *
 * Created:  11-Jan-90, Peter Cherna
 * Modified: 11-Jan-90, Peter Cherna
 *
 */

BOOL AddPrefsEntry(struct MinList *list, ULONG ID,
    WORD ViewPosX, WORD ViewPosY, WORD TxtX, WORD TxtY, struct Rectangle *Std)
{
    struct PrefsEntry *pentry;

    if (pentry = AllocMem(sizeof(struct PrefsEntry), MEMF_CLEAR))
    {
	pentry->ID = ID;
	pentry->ViewPos.x = ViewPosX;
	pentry->ViewPos.y = ViewPosY;
	pentry->Txt.x = TxtX;
	pentry->Txt.y = TxtY;
	pentry->Std = *Std;

	AddTail((struct List *)list, (struct Node *)pentry);
	return(TRUE);
    }
    return(FALSE);
}


/*------------------------------------------------------------------------*/

/*/ BuildCurrentList()
 *
 * Builds the list of PrefsEntry's representing the current state.
 *
 * Created:  11-Jan-90, Peter Cherna
 * Modified: 11-Jan-90, Peter Cherna
 *
 */

BOOL BuildCurrentList(struct MinList *list)
{
    struct MonitorEntry *mon;
    BOOL ok = TRUE;

    for (mon = (struct MonitorEntry *)monitors.lh_Head;
	(ok) && (mon->me_Node.ln_Succ);
	mon = (struct MonitorEntry *)mon->me_Node.ln_Succ)
    {
	DP(("Adding $%lx\n", mon->me_ID));
	ok = AddPrefsEntry(list,
	    mon->me_ID,
	    mon->me_MonitorInfo.ViewPosition.x,
	    mon->me_MonitorInfo.ViewPosition.y,
	    rectWidth( &mon->me_DimensionInfo.TxtOScan ),
	    rectHeight( &mon->me_DimensionInfo.TxtOScan ),
	    &mon->me_DimensionInfo.StdOScan);
    }
    return(ok);
}


/*------------------------------------------------------------------------*/

/*/ BuildDefaultList()
 *
 * Builds the list of PrefsEntry's representing the defaults.
 *
 */

BOOL BuildDefaultList(struct MinList *list)
{
    struct MonitorEntry *mon;
    BOOL ok = TRUE;

    for (mon = (struct MonitorEntry *)monitors.lh_Head;
	(ok) && (mon->me_Node.ln_Succ);
	mon = (struct MonitorEntry *)mon->me_Node.ln_Succ)
    {
	ok = AddPrefsEntry(list,
	    mon->me_ID,
	    mon->me_MonitorInfo.DefaultViewPosition.x,
	    mon->me_MonitorInfo.DefaultViewPosition.y,
	    rectWidth( &mon->me_DimensionInfo.Nominal ),
	    rectHeight( &mon->me_DimensionInfo.Nominal ),
	    &mon->me_DimensionInfo.Nominal);
    }
    return(ok);
}

/*------------------------------------------------------------------------*/

/*/ FreePrefsEntries()
 *
 *
 */

void FreePrefsEntries(struct MinList *list)

    {
    struct PrefsEntry *pentry;

    while (!Empty((struct List *)list))
	{
	pentry = (struct PrefsEntry *)RemHead((struct List *)list);
	FreeMem(pentry, sizeof(struct PrefsEntry));
	}
    }

/*------------------------------------------------------------------------*/

/*/ MapDefaultMonitor()
 *
 * For a MinList of PrefsEntry's, replace all references to the monitor
 * of 'fromid' with 'toid'.  Used to convert default to explicit NTSC/PAL
 * and vice versa.
 *
 * Created:  22-May-90, Peter Cherna
 * Modified: 22-May-90, Peter Cherna
 *
 */

void MapDefaultMonitor(list, fromid, toid)

    struct MinList *list;
    ULONG fromid;
    ULONG toid;

    {
    struct PrefsEntry *pentry;
    struct PrefsEntry *succ;

    DP(("MDM: %lx -> %lx\n", fromid, toid));
    for (pentry = (struct PrefsEntry *)list->mlh_Head;
	succ = (struct PrefsEntry *)pentry->Node.mln_Succ;
	pentry = succ)
	{
	if (MONITOR_PART(pentry->ID) == fromid)
	    {
	    DP(("MDM: Replacing %lx by %lx\n", pentry->ID,
		(pentry->ID & ~MONITOR_ID_MASK) | toid));
	    pentry->ID = (pentry->ID & ~MONITOR_ID_MASK) | toid;
	    }
	}
    }

/*------------------------------------------------------------------------*/
