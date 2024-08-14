/******************************************************************************
*
*   $Id: find_tag.c,v 39.1 92/01/21 13:34:09 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* find_tag.c -- search a record for a specific tag */

#include    "/displayinfo_internal.h"
#include	"d.protos"
#include "/macros.h"
#include <pragmas/utility_pragmas.h>

/***** display.library/AddTag ************************************************
*
*   NAME
*       AddTag -- add a specific TagItem to a tag array
*
*   SYNOPSIS
*       item = AddTag( tags, newitem)
*       d0             a1    a0    
*
*   FUNCTION
*
*   INPUTS
*       tags - pointer to a tag array
*       newitem - tag item to add to array
*
*   RESULT
*       item - pointer to a TagItem with that tag ID or NULL if add failed.
*
*   BUGS
*
*   SEE ALSO
*       utility/tagitem.h
*
*******************************************************************************/

struct TagItem __regargs *add_tag( tags, newitem )
struct TagItem  tags[];
struct TagItem  *newitem; 
{
    struct TagItem *item = NULL;
    ULONG  id = newitem->ti_Tag;

    if( tags && newitem )
    {
	item = tags;

	while( item )
	{
	    if( item->ti_Tag == id) break;

	    switch( item->ti_Tag )
	    {
		case( TAG_DONE ):   /* no more */
		{
		    item->ti_Data = (ULONG)newitem; 
		    item->ti_Tag = TAG_MORE; 
		}   break;

		case( TAG_SKIP ):   /* skip data items */
		{
		    item += (item->ti_Data + 1);            
		}   break;

		case( TAG_MORE ):   /* next array */
		{
		    item = (struct TagItem *)item->ti_Data;     
		}   break;

		case( TAG_IGNORE ): /* ignore data item */
		default:            /* next item */
		{
		    item++;
		}   break;
	    } 
	}
    }

    return( item );
}
