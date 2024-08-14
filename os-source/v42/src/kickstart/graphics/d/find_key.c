/******************************************************************************
*
*   $Id: find_key.c,v 42.0 93/06/16 11:18:01 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* find_key.c -- search a record and its siblings for a specific key */

#include    <exec/types.h>
#include    <exec/memory.h>
#include    "/view.h"
#include    "/gfxbase.h"
#include    "/displayinfo_internal.h"
#include    "d.protos"
#include    "/macros.h"
#include    "/gfxpragmas.h"
#include    <pragmas/utility_pragmas.h>

#include    "/displayinfo.h"
#include    "/displayinfo_internal.h"

/***** display.library/FindKey ***********************************************
*
*   NAME
*       FindKey -- search for a record identified by a specific key
*
*   SYNOPSIS
*       match = FindKey( record, key, mask )
*       d0                a1   d1:16 d0:16
*
*   FUNCTION
*
*   INPUTS
*       record - pointer to a Record
*       key  - unsigned word identifier
*       mask - unsigned word template for which bits to respect in key
*
*   RESULT
*       match - pointer to a display Record with that key or NULL if no match.
*
*   BUGS
*
*   SEE ALSO
*
*******************************************************************************/

struct DisplayInfoRecord __regargs *find_key( match, key, mask )
struct DisplayInfoRecord *match;
UWORD           key;
UWORD           mask;
{
    UWORD       match_key = ( key & mask );

    Forbid();

    for( ; match; match = (struct DisplayInfoRecord *)match->rec_Node.rcn_Succ )
    {
	if( match_key  == (match->rec_MinorKey & mask) ) break;
    }

    Permit();

    return( match );
}

ULONG __regargs MakeTrueID(struct DisplayInfoRecord *root, ULONG ID)
{
    ULONG t = ID;
    UWORD m;

    /* This will attempt to return the true value of DEFAULT monitors.
     * DEFAULT monitors are either NTSC, PAL, DBLNTSC or DBLPAL, depending
     * on the value in GBASE->monitor_id, set in SetDefaultMonitor().
     */

    if (root == GBASE->DisplayInfoDataBase)
    {
	UWORD tID = (ID >> 16);

	ObtainSemaphore(GBASE->MonitorListSemaphore);
	
#ifdef USE_BESTMODE
	/* Should really use this, but is too slow */
	/* How about the default? */
	if ((tID == (DEFAULT_MONITOR_ID >> 16)) && (m = GBASE->monitor_id))
	{
		ULONG best;
		t |= ((m << 16) | 0x1000);
		if ((m == (DBLNTSC_MONITOR_ID >> 16)) || (m == (DBLPAL_MONITOR_ID >> 16)))
		{
			tags[0].ti_Data = ((m << 16) | 0x1000);
			tags[1].ti_Data = (ID | ((m == (DBLNTSC_MONITOR_ID >> 16)) ? NTSC_MONITOR_ID : PAL_MONITOR_ID));
			if ((best = BestModeIDA(tags)) != INVALID_ID)
			{
				t = best
			}
			else
			{
				t = tags[1].ti_Data;	/* use plain old NTSC/PAL */
			}
		}
	}
#else
	/* How about the default? */
	if ((tID == (DEFAULT_MONITOR_ID >> 16)) && (m = GBASE->monitor_id))
	{
		t |= ((m << 16) | 0x1000);
	}
#endif
	tID = (t >> 16);
	/* Any extra fudging we need? */
	if ((tID == (DBLNTSC_MONITOR_ID >> 16)) || (tID == (DBLPAL_MONITOR_ID >> 16)))
	{
		/* Dbl... modes don't support SuperHires, nor can
		 * we promote the ScanDbl modes.
		 */
		t &= ~(SUPERHIRES | DOUBLESCAN);
	}

	ReleaseSemaphore(GBASE->MonitorListSemaphore);
    }

    return(t);
}

/***** display.library/FindID ***********************************************
*
*   NAME
*       FindID -- find a record given its display identifier
*
*   SYNOPSIS
*       record = FindID( root, ID)
*       d0               a1        d0     
*
*   FUNCTION
*
*   INPUTS
*       root - pointer to a Display DataBase RecordNode ("effective root")
*       ID       - unsigned long display ID
*
*   RESULT
*       record   - pointer to a record associate with this ID or NULL
*
*   BUGS
*
*   SEE ALSO
*       NextID(), display/record.h
*
*******************************************************************************/

struct DisplayInfoRecord __regargs *find_id( root, ID )
struct DisplayInfoRecord *root;
ULONG   ID;
{
    struct DisplayInfoRecord *type = NULL;
    struct DisplayInfoRecord *dinfo = NULL;
    UWORD major;
    UWORD minor;

    major  = ID >> 16;
    minor  = ID & 0xFFFF;

    if(ID != INVALID_ID)
    {
	if(root && (type = find_key( (struct DisplayInfoRecord *)(SubRecord(root)), major, (UWORD)~0 )) )
	{
	    dinfo = find_key( (struct DisplayInfoRecord *)(SubRecord(type)), minor, (UWORD)~0 );
	    return( dinfo );
	}
    }

    return( NULL );
}

ULONG no_copy() 
{
    return(0);
}

ULONG get_copy(record,src,size,tagID,ID,dst,bytes)
struct DisplayInfoRecord **record;
UBYTE **src;
ULONG *size;
ULONG *tagID;
ULONG *ID;
UBYTE **dst;
ULONG *bytes;
{
    ULONG result = 0;
    ULONG raw = 0;
    ULONG *stupidcompiler = 0;

    switch(*tagID)
    {
	case( DTAG_DISP ):
	{
	    raw = (ULONG)&(((struct RawDisplayInfo *)stupidcompiler)->reserved[0]);
	}   break;
	case( DTAG_DIMS ):
	{
	    result = get_dims(record,src,size,tagID,ID,dst,bytes);
	}   break;
	case( DTAG_MNTR ):
	{
	    raw = (ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->reserved[0]);
	}   break;
	case( DTAG_NAME ):
	{
	    raw = (ULONG)&(((struct RawNameInfo *)stupidcompiler)->reserved[0]);
	}   break;
	case( DTAG_VEC ):
	{
	    raw = (ULONG)&(((struct RawVecInfo *)stupidcompiler)->reserved[0]);
	}   break;
	default:
	{
	}   break;
    }

    if (raw)
    {
	result = copy_dbstuff(src, size, dst, bytes, raw);
    }

    return( result );
}

ULONG set_copy(record,src,size,tagID,ID,dst,bytes)
struct DisplayInfoRecord **record;
UBYTE **src;
ULONG *size;
ULONG *tagID;
ULONG *ID;
UBYTE **dst;
ULONG *bytes;
{
    ULONG result = 0;
    ULONG raw = 0;
    ULONG *stupidcompiler = 0;

    switch(*tagID)
    {
	case( DTAG_DISP ):
	{
	    raw = (ULONG)&(((struct RawDisplayInfo *)stupidcompiler)->reserved[0]);
	}   break;
	case( DTAG_DIMS ):
	{
	    result = set_dims(record,src,size,tagID,ID,dst,bytes);
	}   break;
	case( DTAG_MNTR ):
	{
	    result = set_mntr(record,src,size,tagID,ID,dst,bytes);
	}   break;
	case( DTAG_NAME ):
	{
	    raw = (ULONG)&(((struct RawNameInfo *)stupidcompiler)->reserved[0]);
	}   break;
	case( DTAG_VEC ):
	{
	    raw = (ULONG)&(((struct RawVecInfo *)stupidcompiler)->reserved[0]);
	}   break;
	default:
	{
	}   break;
    }

    if (raw)
    {
	result = copy_dbstuff(src, size, dst, bytes, raw);
    }

    return( result );
}

ULONG raw_copy(record,src,size,tagID,ID,dst,bytes)
struct DisplayInfoRecord **record;
UBYTE **src;
ULONG *size;
ULONG *tagID;
ULONG *ID;
UBYTE **dst;
ULONG *bytes;
{
    return(copy_dbstuff(src, size, dst, bytes, *size));
}

ULONG __regargs cook( root, handle, buf, size, tagID, ID, mode )
struct DisplayInfoRecord *root;
struct DisplayInfoRecord *handle;
UBYTE buf[];
ULONG size;
ULONG tagID;
ULONG ID;
ULONG mode;
{
    ULONG result = 0;
    ULONG (*sfunc)() = no_copy; /* system function */
    ULONG (*ufunc)() = NULL; 	/* no usr function */
    struct DisplayInfoRecord *dinfo = handle;

    if(buf)
    {
	if((dinfo) || (dinfo = (struct DisplayInfoRecord *)gfx_FindDisplayInfo(ID)))
	{
	    struct DisplayInfoRecord *record = dinfo;
	    struct QueryHeader *query = NULL; 

	    switch( tagID )
	    {
		case( DTAG_MNTR ):
		{
		    /* monitor is global to a family of modes */
		    record = (struct DisplayInfoRecord *)(dinfo->rec_Node.rcn_Parent);
		}   break;
	    }

	    switch( mode )
	    {
		case( GET_DISPLAYINFODATA ):
		{
		    sfunc = get_copy;
		    ufunc = record->rec_get_data;
		}   break;
		case( SET_DISPLAYINFODATA ):
		{
		    sfunc = set_copy;
		    ufunc = record->rec_set_data;
		}   break;
		case( NEW_DISPLAYINFODATA ):
		{
		    sfunc = raw_copy;
		}   break;
	    }

	    if(record) /* effective destination */
	    {
		UBYTE *src;
		UBYTE *dst;
		ULONG  bytes = ( sizeof( struct TagItem ) );

		query = (struct QueryHeader *)FindTagItem(tagID,&record->rec_Tag);

		switch( mode )
		{
		    case( GET_DISPLAYINFODATA ):
		    {
			src = (UBYTE *)query;
			dst = (UBYTE *)buf;
		    }   break;
		    case( SET_DISPLAYINFODATA ):
		    {
			src = (UBYTE *)buf;
			dst = (UBYTE *)query;
		    }   break;
		    case( NEW_DISPLAYINFODATA ):
		    {
			src = (UBYTE *)buf;
			if(!(dst = (UBYTE *)query))
			{	
			    dst = (UBYTE *)AllocMem(size, MEMF_PUBLIC|MEMF_CLEAR);
			    query = (struct QueryHeader *)dst;
			}
		    }   break;
		}

		if(src && dst)
		{
		    int chunk = FALSE;

		    if(((struct QueryHeader*)src)->SkipID == TAG_SKIP)
		    {
			chunk  = TRUE;
			bytes += (sizeof(*query)
			      +(((struct QueryHeader*)src)->Length<<3));
		    }

		    /* call the system function first */
		    {
			UBYTE *a = src;
			ULONG  b = size;
			UBYTE *c = dst;
			ULONG  d = bytes;
			
			result = 
			(*sfunc)(&dinfo,&src,&size,&tagID,&ID,&dst,&bytes);

			/* now call the user function, if any */

			if(ufunc)
			{
			    (*ufunc)(&dinfo,&a,&b,&tagID,&ID,&c,&d,&result);
			}
		    }

		    switch( mode )
		    {
			case( NEW_DISPLAYINFODATA ):
			{
			    if (chunk)
			    {
				query->DisplayID =  /* set dstID */
				*((ULONG *)(&dinfo->rec_MajorKey));
			    }
			    add_tag( &record->rec_Tag, query );
			}   break;
		    }
		}
	    }
	}
    }

    return( result );
}


