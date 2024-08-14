/******************************************************************************
*
*   $Id: find_info.c,v 42.0 93/06/16 11:18:12 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* find_info.c -- search database for displayinfo record by ID */

#include	<exec/types.h>
#include	"/gfxbase.h"
#include	<graphics/view.h>
#include	"/macros.h"
#include	"/displayinfo_internal.h"
#include	"d.protos"
#include 	<pragmas/utility_pragmas.h>

/*#define DEBUG*/
/*#define USEGBDEBUG*/

#ifdef USEGBDEBUG
#define GBDEBUG if (GBASE->Debug)
#else
#define GBDEBUG
#endif

#ifdef DEBUG
#define D(x) {GBDEBUG {x};}
#else
#define D(x)
#endif

/****** graphics.library/FindDisplayInfo ****************************************
*
*   NAME
*	FindDisplayInfo -- search for a record identified by a specific key (V36)
*
*   SYNOPSIS
*	handle = FindDisplayInfo(ID)
*	D0                       D0
*
*	DisplayInfoHandle FindDisplayInfo(ULONG);
*
*   FUNCTION
*	Given a 32-bit Mode Key, return a handle to a valid DisplayInfoRecord
*	found in the graphics database.  Using this handle, you can obtain
*	information about this Mode, including its default dimensions,
*	properties, and whether it is currently available for use.
*
*   INPUTS
*	ID     - unsigned long identifier
*
*   RESULT
*	handle - handle to a displayinfo Record with that key
*	         or NULL if no match.
*
*   BUGS
*
*   SEE ALSO
*	graphics/displayinfo.h
*
*******************************************************************************/

void all_db_avail(void);

struct DisplayInfoRecord * __asm FindDisplayInfo( register __d0 ULONG ID)
{
    extern UBYTE PalDisplayTags[];
    extern UBYTE NtscDisplayTags[];
    struct DisplayInfoRecord *dinfo, *head, *head2 = NULL;
    struct RawDisplayInfo *rdi;
    UBYTE *tags = PalDisplayTags;
    struct DisplayInfoRecord *dib;

	dib = (struct DisplayInfoRecord *)GBASE->DisplayInfoDataBase;
	ID = MakeTrueID(dib, ID);
	if (head = find_key((struct DisplayInfoRecord *)(SubRecord(dib)), (ID >> 16), (UWORD)~0))
	{
		head2 = find_key((SubRecord(head)), (ID & 0xffff), ~0);
	}

	D(kprintf("finddisplayinfo(). ID = 0x%lx, dib = 0x%lx, head = 0x%lx, head2 = 0x%lx\n", ID, dib, head, head2);)

	if ((dinfo = find_id(dib, ID)) == NULL)
	{
		/* could not find this ID. Was it legal? */
		D(kprintf("Could not find ID\n");)
		if (((ID != INVALID_ID) && ((ID & EXTENDED_MODE) && (ID & 0xFFFF0000))) ||
		    ((ID & MONITOR_ID_MASK) == DEFAULT_MONITOR_ID))
		{
		/* OK - it was legal. Now, could we not find it because
		 * the mode was not legal for this monitor, or because
		 * the monitor has not been unpacked?
		*/
			D(kprintf("mode was legal\n");)
			if (head)
			{
			/* Have the head of the monitor. Is this an invalid ID
			 * for a disk-based monitor that has not been made available
			 * yet?
			*/
				D(kprintf("have head\n");)
				/* look for the RawDisplayInfo */
				if (head2 && ((!(rdi = (struct RawDisplayInfo *)FindTagItem(DTAG_DISP, &head2->rec_Tag))) ||
				    (rdi->NotAvailable & DI_AVAIL_NOMONITOR)))
				{
				/* make it available */
					D(kprintf("db_a() 1\n");)
					db_avail(head2);
				}
				/* Either way, this is an invalid mode id. */
				return(NULL);
			}
			else
			{
			/* No head. Unpack from ROM to RAM and try again */
				D(kprintf("no head - unpacking\n");)
				if (!(GBASE->DisplayFlags & PAL))
				{
					tags = NtscDisplayTags;
				}
				do_db_startup(dib, tags, ((ID & MONITOR_ID_MASK) >> 16));
				D(kprintf("db_a() 2, ");)
				head = find_key((struct DisplayInfoRecord *)(SubRecord(dib)), (ID >> 16), (UWORD)~0);
				D(kprintf("head = 0x%lx\n", head);)
				all_db_avail();
				return( find_id(dib, ID ) );
			}
		}
		else
		{
		/* Not a legal mode id. */
			D(kprintf("illegal modeid\n");)
			return(NULL);		
		}
	}
	else
	{
	/* Found the ID. If the monitor has been added from disk, then we need
	 * to make the monitor available if it has not already been done.
	*/
		D(kprintf("found the ID\n");)
		if (head)
		{
		/* look for the RawDisplayInfo */
			D(kprintf("have the head\n");)
			if (head2 && ((rdi = (struct RawDisplayInfo *)FindTagItem(DTAG_DISP,&head2->rec_Tag)) &&
			    (rdi->NotAvailable & DI_AVAIL_NOMONITOR)))
			{
			/* make it available */
				D(kprintf("making it available\n");)
				D(kprintf("db_a() 3\n");)
				db_avail(head2);
			}
		}
		else
		{
			D(kprintf("should never reach here!\n");)
		}			 
		D(kprintf("returning dinfo = 0x%lx\n", dinfo);)
		return(dinfo);
	}
}

/****** graphics.library/NextDisplayInfo ***************************************
*
*   NAME
*	NextDisplayInfo -- iterate current displayinfo identifiers (V36)
*
*   SYNOPSIS
*	next_ID = NextDisplayInfo(last_ID)
*	D0                        D0
*
*	ULONG NextDisplayInfo(ULONG);
*
*   FUNCTION
*	The basic iteration function with which to find all records in the
*	graphics database.  Using each ID in succession, you can then call
*	FindDisplayInfo() to obtain the handle associated with each ID.
*	Each ID is a 32-bit Key which uniquely identifies one record.
*	The INVALID_ID is special, and indicates the end-of-list.
*
*   INPUTS
*	last_ID - previous displayinfo identifier
*	          or INVALID_ID if beginning iteration.
*
*   RESULT
*	next_ID - subsequent displayinfo identifier
*	          or INVALID_ID if no more records.
*
*   BUGS
*
*   SEE ALSO
*	FindDisplayInfo(), GetDisplayInfoData()
*	graphics/displayinfo.h
*
*******************************************************************************/

ULONG __asm NextDisplayInfo( register __d0 ULONG ID)
{
	ULONG next = ID;
	ULONG skip = INVALID_ID;

#ifdef SKIP_ID
	if( GBASE->DisplayFlags & NTSC ) skip = NTSC_MONITOR_ID;
	if( GBASE->DisplayFlags & PAL  ) skip =  PAL_MONITOR_ID;
#endif

	while(( next = next_id( GBASE->DisplayInfoDataBase, next )) != INVALID_ID )
	{
	    if( (next & skip ) != skip )
	    {
		break;
	    }
	}

	return( next );
}

/****i* graphics.library/AddDisplayInfo ****************************************
*
*   NAME
*       AddDisplayInfo -- add a Record to a Display DataBase (V36)
*
*   SYNOPSIS
*       handle = AddDisplayInfo( record )
*       d0                       a0
*
*   FUNCTION
*
*   INPUTS
*       record  - pointer to a DisplayInfoRecord to link to the Display DataBase
*
*   RESULT
*       handle -   handle to this Record after linking or NULL if link failed.
*
*   BUGS
*       does not check to see if there is already a child with identical key.
*
*   SEE ALSO
*       graphics/displayinfo.h
*
*******************************************************************************/

struct DisplayInfoRecord * __asm AddDisplayInfo( register __a0 struct DisplayInfoRecord  *record)
{
	return( add_record( GBASE->DisplayInfoDataBase, (struct RecordNode *)record ) );
}

/****** graphics.library/GetDisplayInfoData **************************************
*
*   NAME
*	GetDisplayInfoData -- query DisplayInfo Record parameters (V36)
*
*   SYNOPSIS
*	result = GetDisplayInfoData(handle, buf, size, tagID, [ID])
*	D0                          A0      A1   D0    D1     [D2]
*
*	ULONG GetDisplayInfoData(DisplayInfoHandle, UBYTE *, ULONG, ULONG, ULONG);
*
*   FUNCTION
*	GetDisplayInfoData() fills a buffer with data meaningful to the
*	DisplayInfoRecord pointed at by your valid handle. The data type
*	that you are interested in is indicated by a tagID for that chunk.
*	The types of tagged information that may be available include:
*
*	DTAG_DISP: (DisplayInfo)   - properties and availability information.
*	DTAG_DIMS: (DimensionInfo) - default dimensions and overscan info.
*	DTAG_MNTR: (MonitorInfo)   - type, position, scanrate, and compatibility
*	DTAG_NAME: (NameInfo)      - a user friendly way to refer to this mode.
*
*   INPUTS
*	handle - displayinfo handle
*	buf    - pointer to destination buffer
*	size   - buffer size in bytes
*	tagID  - data chunk type
*	ID     - displayinfo identifier, optionally used if handle is NULL
*
*   RESULT
*	result - if positive, number of bytes actually transferred
*	         if zero, no information for ID was available
*
*   BUGS
*
*   SEE ALSO
*	FindDisplayInfo(), NextDisplayInfo()
*	graphics/displayinfo.h
*
*******************************************************************************/

struct DisplayInfoRecord * __asm GetDisplayInfoData( register __a0 struct DisplayInfoRecord *handle,
				register __a1 UBYTE *buf,
				register __d0 ULONG size,
				register __d1 ULONG tagID,
				register __d2 ULONG ID)
{
	return((struct DisplayInfoRecord *)cook(GBASE->DisplayInfoDataBase, handle, buf, size, tagID, ID, GET_DISPLAYINFODATA)); 
}

/***** graphics.library/AddDisplayInfoData ************************************
*
*   NAME
*       AddDisplayInfoData -- add data chunk to DisplayInfo Record
*
*   SYNOPSIS
*       result = AddDisplayInfoData( handle, buf, size, tagID, [ID] )
*       d0                	     a0      a1   d0    d1	d2 
*	
*	DisplayInfoHandle handle;
*	UBYTE   buf[];
*	ULONG   size;
*	ULONG   tagID;
*	ULONG   ID;
*	
*	ULONG  result;
*
*   FUNCTION
*
*   INPUTS
*	handle - displayinfo handle
*	buf    - pointer to destination buffer
*	size   - buffer size in bytes
*	tagID  - data chunk type
*       ID     - displayinfo identifier, optionally used if handle is NULL
*
*   RESULT
*       result - if positive, number of bytes actually added
*		 if zero, no data was actually transferred 
*
*   BUGS
*
*   SEE ALSO
*	GetDisplayInfoData(), SetDisplayInfoData()
*	graphics/displayinfo.h
*
*******************************************************************************/

struct DisplayInfoRecord * __asm AddDisplayInfoData(register __a0 struct DisplayInfoRecord *handle,
				register __a1 UBYTE *buf,
				register __d0 ULONG size,
				register __d1 ULONG tagID,
				register __d2 ULONG ID)
{
	return((struct DisplayInfoRecord *)cook(GBASE->DisplayInfoDataBase, handle, buf, size, tagID, ID, NEW_DISPLAYINFODATA)); 
}

/***** graphics.library/SetDisplayInfoData ************************************
*
*   NAME
*       SetDisplayInfoData -- change DisplayInfo Record parameters 
*
*   SYNOPSIS
*       result = SetDisplayInfoData( handle, buf, size, tagID, [ID] )
*       d0                	     a0      a1   d0    d1	d2 
*	
*	DisplayInfoHandle handle;
*	UBYTE   buf[];
*	ULONG   size;
*	ULONG   tagID;
*	ULONG   ID;
*	
*	ULONG  result;
*
*   FUNCTION
*
*   INPUTS
*	handle - displayinfo handle
*	buf    - pointer to destination buffer
*	size   - buffer size in bytes
*	tagID  - data chunk type
*       ID     - displayinfo identifier, optionally used if handle is NULL
*
*   RESULT
*       result - if positive, number of bytes actually transferred
*		 if zero, no data was actually transferred 
*
*   BUGS
*
*   SEE ALSO
*	FindDisplayInfo(), NextDisplayInfo(), GetDisplayInfoData()
*	graphics/displayinfo.h
*
*******************************************************************************/

struct DisplayInfoRecord * __asm SetDisplayInfoData(register __a0 struct DisplayInfoRecord *handle,
				register __a1 UBYTE *buf,
				register __d0 ULONG size,
				register __d1 ULONG tagID,
				register __d2 ULONG ID)
{
	D(kprintf("SetDisplayInfoData(). Record = 0x%lx, ID = 0x%lx\n", handle, ID));
	return((struct DisplayInfoRecord *)cook(GBASE->DisplayInfoDataBase, handle, buf, size, tagID, ID, SET_DISPLAYINFODATA));
}
