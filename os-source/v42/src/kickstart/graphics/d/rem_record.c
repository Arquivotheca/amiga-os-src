/******************************************************************************
*
*   $Id: rem_record.c,v 42.0 93/06/16 11:18:08 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* rem_record.c -- remove a record from a display */

#include    "/displayinfo_internal.h"
#include	"d.protos"

/***** display.library/RemRecord *********************************************
*
*   NAME
*       RemRecord -- remove a Record from a Display DataBase
*
*   SYNOPSIS
*       node = RemRecord( record )
*       d0                a0
*
*   FUNCTION
*
*   INPUTS
*       record - pointer to a RecordNode to delink from this Display DataBase.
*
*   RESULT
*       node - pointer to this RecordNode after delinking.
*
*   BUGS
*
*   NOTE
*       will remove a record and leave all its children attached but
*       will clear all this record's links to its parent and siblings.
*
*   SEE ALSO
*       display/record.h
*
*******************************************************************************/

#ifdef UNDEFINED

struct DisplayInfoRecord *rem_record( record )
struct RecordNode *record;
{
    if( record )
    {
	struct RecordNode  *parent  = record->rcn_Parent;
	struct RecordNode  *sibling = record->rcn_Pred;

	Forbid();

	/* leave subtree attached to the removed node */

	if( parent && (parent->rcn_Child == record) ) 
	{
	    parent->rcn_Child = record->rcn_Succ; /* child == first sibling */
	}
	else
	{
	    if( sibling )                          /* child != first sibling */
	    {
		sibling->rcn_Succ = record->rcn_Succ;
		if( record->rcn_Succ )
		{
		    record->rcn_Succ->rcn_Pred = sibling;
		}
	    }
	}

	record->rcn_Parent  = NULL; 
	record->rcn_Pred    = NULL;
	record->rcn_Succ    = NULL;

	Permit();
    }

    return( record );
}

#endif
