/******************************************************************************
*
*   $Id: next_record.c,v 42.0 93/06/16 11:18:10 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* next_record.c -- iterate available records */

#include    "/displayinfo_internal.h"
#include	"d.protos"

/***** display.library/NextRecord ********************************************
*
*   NAME
*       NextRecord -- find successive records
*
*   SYNOPSIS
*       next = NextRecord( root, current )
*       d0                 a1        a0       
*
*   FUNCTION
*
*   INPUTS
*       root - pointer to a Display DataBase RecordNode ("effective root")
*       current  - pointer to a current RecordNode or NULL
*
*   RESULT
*       next - pointer to the subsequent Record or NULL
*
*   BUGS
*
*   SEE ALSO
*       display/record.h
*
*******************************************************************************/

struct DisplayInfoRecord __regargs *next_record( root, record )
struct DisplayInfoRecord *root;
struct DisplayInfoRecord *record;
{
    struct DisplayInfoRecord *next = NULL;

    if( record ) next = find_key((struct DisplayInfoRecord *)(SibRecord(record)), 0, 0 );
    else if( root ) next = find_key((struct DisplayInfoRecord *)(SubRecord(root)), 0, 0 );

    return( next );
}

/***** display.library/NextID ********************************************
*
*   NAME
*       NextID -- find successive display identifiers
*
*   SYNOPSIS
*       nextID = NextID( root, currID)
*       d0               a1        d0     
*
*   FUNCTION
*
*   INPUTS
*       root - pointer to a Display DataBase RecordNode ("effective root")
*       currID   - current unsigned long display ID or NULL
*
*   RESULT
*       nextID   - subsequent unsigned long display ID or NULL
*
*   BUGS
*
*   SEE ALSO
*       FindID(), display/record.h
*
*******************************************************************************/

ULONG __regargs next_id( root, ID )
struct DisplayInfoRecord *root;
ULONG   ID;
{
	UWORD major  = ID >> 16;
	UWORD minor  = ID & 0xFFFF;
	struct DisplayInfoRecord *type = NULL;
	struct DisplayInfoRecord *object = NULL;
	UWORD mask = ((ID == INVALID_ID) ? 0 : ~0);

	if(root)
	{
	    if( type = find_key((struct DisplayInfoRecord *)(SubRecord(root)),major,mask))
	    {
		object = (struct DisplayInfoRecord *)(SubRecord(type));

		if(mask) 
		{
		    object = next_record(type,find_key(object,minor,mask));
		}
	    }

	    if(!object && type && (type = find_key((struct DisplayInfoRecord *)(SibRecord(type)),0,0)))
	    {
		object = find_key((struct DisplayInfoRecord *)(SubRecord(type)),0,0);
	    }
	}

	if( object ) 
	{
	    return((ULONG)(((struct DisplayInfoRecord *) object->rec_Node.rcn_Parent)->rec_MinorKey<<16 | object->rec_MinorKey));
	}
	else
	{
	    return( INVALID_ID );
	}
}
