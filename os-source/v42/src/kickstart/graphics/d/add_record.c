/******************************************************************************
*
*   $Id: add_record.c,v 42.0 93/06/16 11:18:06 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* add_record.c -- add a record to a display */

#include "/macros.h"
#include    "/displayinfo_internal.h"
#include	"d.protos"
#include <pragmas/exec_pragmas.h>

/***** display.library/AddRecord *********************************************
*
*   NAME
*       AddRecord -- add a Record to a Display DataBase
*
*   SYNOPSIS
*       child = AddRecord( root, record )
*       d0                 a1       a0    
*
*   FUNCTION
*
*   INPUTS
*       root - pointer to a Display DataBase RecordNode ("effective root")
*       record   - pointer to a RecordNode to link to this Display DataBase.
*
*   RESULT
*       child - pointer to this Record after linking or NULL if the link failed.
*
*   BUGS
*       does not check to see if there is already a child with identical key.
*
*   SEE ALSO
*       display/record.h
*
*******************************************************************************/

struct DisplayInfoRecord __regargs *add_record( root, record )
struct RecordNode *root;
struct RecordNode *record;
{
    if( root && record )
    {
	struct RecordNode **sibling;
	
	/* does this root already have children? */
	for( sibling = &root->rcn_Child;
	   (*sibling && (*sibling)->rcn_Succ );
	     sibling = &(*sibling)->rcn_Succ );

	Forbid();

	record->rcn_Pred   = NULL;
	record->rcn_Parent = root;

	/* long identifier */
	((struct DisplayInfoRecord*)record)->rec_MajorKey = 
	((struct DisplayInfoRecord*)  root)->rec_MinorKey;

	if( *sibling )  /* one child among many */
	{
	    (*sibling)->rcn_Succ = record; 
	    record->rcn_Pred = (*sibling);
	}
	else            /* only child */
	{
	    root->rcn_Child = record;
	}

	Permit();
    }
    else
    {
	record = NULL;
    }

    return((struct DisplayInfoRecord *)record);
}
