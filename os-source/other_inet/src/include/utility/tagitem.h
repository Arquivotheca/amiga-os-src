#ifndef UTILITY_TAGITEM_H
#define UTILITY_TAGITEM_H
/*
**	$Id: tagitem.h,v 39.1 92/01/20 11:23:27 vertex Exp $
**
**	Extended specification mechanism
**
**	(C) Copyright 1989-1992 Commodore-Amiga Inc.
**	All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/*****************************************************************************/


/* Tags are a general mechanism of extensible data arrays for parameter
 * specification and property inquiry. In practice, tags are used in arrays,
 * or chain of arrays.
 *
 */

typedef ULONG Tag;

struct TagItem
{
    Tag	  ti_Tag;  	/* identifies the type of data */
    ULONG ti_Data;	/* type-specific data          */
};

/* constants for Tag.ti_Tag, system tag values */
#define TAG_DONE   (0L)	  /* terminates array of TagItems. ti_Data unused */
#define TAG_END	   (0L)   /* synonym for TAG_DONE                         */
#define	TAG_IGNORE (1L)	  /* ignore this item, not end of array		  */
#define	TAG_MORE   (2L)	  /* ti_Data is pointer to another array of TagItems
			   * note that this tag terminates the current array
			   */
#define	TAG_SKIP   (3L)	  /* skip this and the next ti_Data items	  */

/* Indication of user tag, OR this in with user tag values */
#define TAG_USER   (1L<<31)  /* differentiates user tags from system tags */

/* NOTE: Until further notice, tag bits 16-30 are RESERVED and should be zero.
 *       Also, the value (TAG_USER | 0) should never be used as a tag value.
 */


/*****************************************************************************/


/* Tag filter logic specifiers for use with FilterTagItems() */
#define TAGFILTER_AND 0		/* exclude everything but filter hits	*/
#define TAGFILTER_NOT 1		/* exclude only filter hits		*/


/*****************************************************************************/


/* Mapping types for use with MapTags() */
#define MAP_REMOVE_NOT_FOUND 0	/* remove tags that aren't in mapList */
#define MAP_KEEP_NOT_FOUND   1	/* keep tags that aren't in mapList   */


/*****************************************************************************/


/* Merging types for use with MergeTagItems() */
#define MERGE_OR_LIST_1   0	/* list 1's item is preferred        */
#define MERGE_OR_LIST_2   1	/* list 2's item is preferred        */
#define MERGE_AND_LIST_1  2	/* item must appear in both lists    */
#define MERGE_AND_LIST_2  3	/* item must appear in both lists    */
#define MERGE_NOT_LIST_1  4	/* item must not appear in list 1    */
#define MERGE_NOT_LIST_2  5	/* item must not appear in list 2    */
#define MERGE_XOR         6	/* item must appear in only one list */


/*****************************************************************************/


#endif /* UTILITY_TAGITEM_H */
