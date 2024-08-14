/*** tagitem.c *****************************************************************
 *
 *  tagitem -- tag list manipulation routines
 *
 *  $Id: tagitem.c,v 36.6 90/11/05 19:02:57 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>

#include "tagitem.h"
#include "ubase.h"

#define UNUSED	(0)

#define D(x)	;
#define printf	kprintf


/****** utility.library/NextTagItem *************************************
*
*    NAME
*	NextTagItem	--  (New for V36)  Iterate TagItem lists
*
*    SYNOPSIS
*	struct TagItem	*NextTagItem( struct TagItem **TagItemPtr )
*						A0
*    FUNCTION
*	Iterates through a (chained) array of TagItem structures,
*	skipping and chaining as dictated by system tags.
*
*    INPUTS
*	TagItemPtr = doubly-indirect reference to a TagItem structure.
*	    For example, if you have a TagItem pointer which is the
*	    beginning of a TagItem list, pass its address to this
*	    function.  The pointer will be changed to keep track
*	    of the iteration.
*	NOTE: Do NOT use the value of *TagItemPtr, but rather use
*	the pointer returned by this function.
*
*    RESULT
*	Each TagItem in the array or chain of arrays that should
*	be processed according to system Tag values (in utility/tagitem.h)
*	is returned in turn with successive calls.
*
*    BUGS
*
*    SEE ALSO
*	utility/tagitem.h, GetTagData(), PackBoolTags(), FindTagItem()
*
*****************************************************************************/

struct TagItem	* __asm
nextTagItem(	register __a0 struct TagItem	**tp )
{
    struct TagItem *nextti;

    D( printf("NTI: here I am %lx\n", tp ) );

    /* 'tp' already holds "next" item in list */
    nextti = *tp;

    /* walk all TAG_MORE and TAG_IGNORE chaining */
    while ( nextti )
    {
	switch ( nextti->ti_Tag )
	{
	case TAG_MORE:
	    nextti = (struct TagItem *) nextti->ti_Data;
	    break;
	case TAG_SKIP:
	    nextti+= 1 + (int) nextti->ti_Data;
	    break;
	case TAG_IGNORE:
	    nextti++;
	    break;
	case TAG_DONE:
	    return ( *tp = NULL );
	    break;
	default:	/* a normal tag item	*/
	    *tp = nextti + 1;
	    return ( nextti );
	}
    }
    return ( *tp = NULL );
}


/****** utility.library/FindTagItem *************************************
*
*    NAME
*	FindTagItem	--  (New for V36) Scans TagItem list for a Tag
*
*    SYNOPSIS
*	struct TagItem	*FindTagItem( Tag TagVal, struct TagItem *TagList );
*					   D0			    A0
*
*    FUNCTION
*	Scans a TagItem "List", which is in fact a chain of arrays
*	of TagItem structures as defined in utility/tagitem.h.
*	Returns a pointer to the FIRST item with ti_Tag matching the
*	'TagVal' parameter.
*
*    INPUTS
*	TagVal = Tag value to search for.
*	TagList = beginning of TagItem list to scan.
*
*    RESULT
*	Returns a pointer to the item with ti_Tag matching 'TagVal'.
*	Returns NULL if there is no match or if TagList is NULL.
*
*    BUGS
*
*    SEE ALSO
*	utility/tagitem.h, GetTagData(), PackBoolTags(), NextTagItem()
*
*****************************************************************************/

struct TagItem	* __asm
findTagItem(  	register __d0 Tag 		tag,
		register __a0 struct TagItem	*ti )
{
    struct TagItem	*tistate;

    tistate = ti;

    while ( ti = nextTagItem( &tistate ) )
    {
	if ( ti->ti_Tag == tag ) break;
    }
    return ( ti );
}

/****** utility.library/GetTagData *************************************
*
*    NAME
*	GetTagData	--  (New for V36) Obtain data corresponding to
*		Tag in TagItem list, or default value if item not found.
*
*    SYNOPSIS
*	ULONG GetTagData( Tag TagVal, ULONG Default, struct TagItem *TagList )
*				D0	       D1			A0
*
*    FUNCTION
*	Searches a TagItem list for a matching Tag value, and returns the
*	corresponding ti_Data value for the TagItem found.  If none
*	found, will return the value passed it as 'Default'.
*
*    INPUTS
*	TagVal = Tag value to search for.
*	Default = Value to be returned if TagVal is not found.
*	TagList = The TagItem list to search.
*
*    RESULT
*	The ti_Data value for first matching TagItem, or 'Default' if
*	ti_Tag matching 'Tag' is not found.
*
*    BUGS
*
*    SEE ALSO
*	utility/tagitem.h, FindTagItem(), PackBoolTags(), NextTagItem()
*
*****************************************************************************/

ULONG __asm
getTagData(  	register __d0 Tag 		tag,
		register __d1 ULONG		deflt,
		register __a0 struct TagItem	*ti )
{
    if ( ti = findTagItem( tag, ti ) )
    {
	return ( ti->ti_Data );
    }
    return ( deflt );
}

/****** utility.library/PackBoolTags *************************************
*
*    NAME
*	PackBoolTags	--  (New for V36) Builds a "Flag" word from the
*		Boolean tags in a TagList.
*
*    SYNOPSIS
*	PackBoolTags( ULONG InitialFlags, struct TagItem *TagList,
*				D0			   A0
*			struct TagItem *BoolMap );	
*					  A1
*
*    FUNCTION
*	Picks out the Boolean TagItems in a TagItem list and converts
*	them into bit-flag representations according to a correspondence
*	defined by the TagItem list 'BoolMap.'
*
*	A Boolean TagItem is one where only the logical value of
*	the ti_Data is relevant.  If this field is 0, the value is
*	FALSE, otherwise TRUE.
*	
*
*    INPUTS
*	InitialFlags = A starting set of bit-flags which will be
*	    changed by the processing of TRUE and FALSE Boolean
*	    tags in TagList.
*	TagList = A TagItem list which may contain several TagItems
*	    defined to be "Boolean" by their presence in BoolMap.
*	    The logical value of ti_Data determines whether a TagItem
*	    causes the bit-flag value related by BoolMap to set or
*	    cleared in the returned flag longword.
*	BoolMap = A TagItem list defining the Boolean Tags to be
*	    recognized, and the bit (or bits) in the returned longword
*	    that are to be set or cleared when a Boolean Tag is
*	    found to be TRUE or FALSE in TagList.
*
*    RESULT
*	The accumulated longword of bit-flags, starting with InitialFlags
*	and modified by each Boolean TagItem encountered.	
*
*    EXAMPLE
*
*    define some nice user tag values ...
*	enum mytags { tag1 = TAG_USER+1, tag2, tag3, tag4, tag5 };
*
*    this TagItem list defines the correspondence between Boolean tags
*    and bit-flag values.
*	struct TagItem	boolmap[] = {
*	    { tag1,  0x0001 },
*	    { tag2,  0x0002 },
*	    { tag3,  0x0004 },
*	    { tag4,  0x0008 },
*	    { TAG_DONE }
*	};
*
*    You are probably passed these by some client, and you want
*    to "collapse" the Boolean content into a single longword.
*
*	struct TagItem	boolexample[] = {
*	    { tag1,  TRUE },
*	    { tag2,  FALSE },
*	    { tag5, Irrelevant },
*	    { tag3,  TRUE },
*	    { TAG_DONE }
*	};
*
*    Perhaps 'boolflags' already has a current value of 0x800002.
*	boolflags = PackBoolTags( boolflags, boolexample, boolmap );
*
*	The resulting new value of 'boolflags' will be 0x80005.
*	
*    BUGS
*	There are some undefined cases if there is duplication of
*	a given Tag in either list.  It is probably safe to say that
*	the *last* of identical Tags in TagList will hold sway.
*
*    SEE ALSO
*	utility/tagitem.h, GetTagData(), FindTagItem(), NextTagItem()
*
*****************************************************************************/

ULONG __asm
packBoolTags(  	register __d0 ULONG		flagval,
		register __a0 struct TagItem	*tlist,
		register __a1 struct TagItem	*boolmap )

{
    struct TagItem	*ti;
    struct TagItem	*tstate;
    ULONG		flag;

    D( kprintf("pBT initial %lx\n", flagval ) );

    tstate = tlist;
    while ( ti = nextTagItem( &tstate ) )
    {
	/* will do arithemetic with 0 flag if nofind in boolmap */
	flag = getTagData( ti->ti_Tag, 0, boolmap )
	D( kprintf("tag %lx maps to %lx ", ti->ti_Tag,flag ) );

	if ( ti->ti_Data )
	{
	    D( kprintf("set the flags\n") );
	    flagval |= flag;
	}
	else
	{
	    D( kprintf("clear the flags\n") );
	    flagval &= ~flag;
	}
	D(kprintf("new flags %lx\n", flagval));
    }
    return ( flagval );
}

#if UNUSED
/*
 * if 'tag' is included in 'boolmap'
 * return find it and return a boolean test of 
 * ti_Data against longword 'accum'
 *
 * ZZZ: might need this to tell you whether the tag was found
 * in the boolmap.
 */
BOOL
unpackBoolTags( accum, tag, boolmap )
Tag	tag;
ULONG	accum;
struct TagItem	*boolmap;
{
    return ( accum & getTagData( tag, 0, boolmap ) );
}
#endif

/****** utility.library/FilterTagChanges *************************************
*
*    NAME
*	FilterTagChanges --  Eliminate TagItems which specify no change.
*
*    SYNOPSIS
*	FilterTagChanges( ChangeList, OldValues, Apply)
*			      A0	A1	  D0
*
*	struct TagItem	*ChangeList;
*	struct TagItem	*OldValues;
*	BOOL		Apply;
*
*    FUNCTION
*	Eliminate items from a "change list" that specify values already
*	in effect in existing list.  Optionally update the existing list
*	if the Boolean 'Apply' is true.
*
*	The elimination is done by changing the ti_Tag field to TAG_IGNORE.
*	So, this function may change the input tag list(s).
*
*    INPUTS
*	ChangeList = specification of new tag-value pairs
*	OldValues = a list of existing tag item pairs
*	Apply = Boolean specification as to whether the values in
*		OldValues are to be updated to the values in ChangeList
*
*    RESULT
*	None
*
*    BUGS
*
*    SEE ALSO
*
*****************************************************************************/

void	__asm
filterTagChanges( register __a0	struct TagItem	*newtl,
		  register __a1 struct TagItem	*oldtl,
		  register __d0	int		apply )
{
    struct TagItem	*ti;
    struct TagItem	*oldti;
    struct TagItem	*tstate;	/* for iteration */

    tstate = newtl;

    while ( ti = nextTagItem( &tstate ) )
    {
	if ( oldti = findTagItem( ti->ti_Tag, oldtl ) )
	{
	    /* match */
	    if ( ti->ti_Data == oldti->ti_Data )
	    {
		ti->ti_Tag = TAG_IGNORE;	/* data values match	*/
	    }
	    else if ( apply )
	    {
		oldti->ti_Data = ti->ti_Data;	/* update old value	*/
	    }
	}
	/* else? ZZZ: do nothing? */
    }
}

/****** utility.library/MapTags *************************************
*
*    NAME
*	MapTags	-- Convert ti_Tag values in a list via map pairing.
*
*    SYNOPSIS
*	MapTags(TagList, MapList, IncludeMiss)
*		  A0	   A1
*
*	struct TagItem	*TagList;
*	struct TagItem	*MapList;
*	BOOL		IncludeMiss;
*
*    FUNCTION
*	Apply a "mapping list" 'MapList' to TagList:
*
*	If the ti_Tag field of an item in 'TagList' appears as ti_Tag
*	in some item in 'MapList', overwrite ti_Tag with the corresponding
*	ti_Data from the map list.
*
*	If a tag in TagList does not appear in the MapList, you can
*	choose to have it removed by changing it to TAG_IGNORE.
*	Do this by setting 'IncludeMiss' to 0.
*
*	If you want to have items which do not appear in the MapList
*	survive the mapping as-is, set IncludeMiss to 1.
*
*	This is central to gadget interconnections where you want
*	to convert the tag values from one space (the sender) to
*	another (the receiver).
*
*    EXAMPLE
*	Consider this source list:
*	    struct TagItem list[] = {
*		{ MY_SIZE, 	71 },
*		{ MY_WEIGHT, 	200 },
*		{ TAG_END,	} };
*
*	And the mapping list:
*	    struct TagItem map[] = {
*		{ MY_SIZE, 	HIS_TALL },
*		{ TAG_END,	} };
*
*	Then after MapTags( list, map, 0 ), 'list' will become:
*		{ HIS_TALL, 71 },
*		{ TAG_IGNORE, },
*		{ TAG_END, }
*
*	Then after MapTags( list, map, 1 ), 'list' will become:
*		{ HIS_TALL, 71 },
*		{ MY_WEIGHT, 200 },
*		{ TAG_END, }
*
*	Note that you can "filter" a list by passing IncludeMiss
*	as 0, and having the data items in the map list equal
*	the corresponding tags.
*
*	You can perform the inverse filter ("everything but")
*	by passing IncludeMiss equal to 1, and creating a
*	map item for every tag you want to filter out, pairing
*	it with a mapped data value of TAG_IGNORE.
*
*	Note: for safety and "order independence" of tag item arrays,
*	if you attempt to map some tag to the value TAG_END, the
*	value TAG_IGNORE will be substituted instead.
*
*    NOTE
*	The procedure will change the values of the input tag list
*	TagList (but not MapList).
*
*    INPUTS
*	TagList = Input list of tag items which is to be mapped
*		to Tag values as specified in MapList.
*	MapList = a "mapping list" TagItem list which pairs Tag
*		values expected to appear in TagList with new
*		values to be substituted in the ti_Tag fields of
*		TagList.  May be NULL, which means that all items
*		in TagList will be eliminated.
*
*    RESULT
*	
*
*    BUGS
*
*    SEE ALSO
*
*****************************************************************************/

ULONG	__asm
mapTags(register __a0	struct TagItem	*tl,
	register __a1	struct TagItem	*map,
	register __d0	int	IncludeMiss )
{
    struct TagItem	*ti;
    struct TagItem	*mapitem;
    struct TagItem	*tstate;	/* for iteration */
    ULONG		count = 0;

    tstate = tl;

    while ( ti = nextTagItem( &tstate ) )
    {
	if ( mapitem = findTagItem( ti->ti_Tag, map ) )
	{
	    /* match */
	    if ( ( ti->ti_Tag = (Tag) mapitem->ti_Data ) == TAG_END )
	    {
	    	/* patch up, protect against bogus termination  */
		ti->ti_Tag = TAG_IGNORE;
	    }
	
	    count++;
	}
	else	/* no mapping */
	{
	    /* eliminate only if he wants	*/
	    if ( IncludeMiss = 0 ) ti->ti_Tag = TAG_IGNORE;
	}
    }

    return ( count );
}

/****** utility.library/AllocateTagItems *************************************
*
*    NAME
*	AllocateTagItems --  Allocate a TagItem array (or chain).
*
*    SYNOPSIS
*	TagList = AllocateTagItems( NumItems )
*			  D0
*
*	struct TagItem	*TagList;
*	ULONG		NumItems;
*
*    FUNCTION
*	Allocates the specified number of usable TagItems slots, and
*	of a format that the function FreeTagItems can handle.
*
*	Note that to access the TagItems in 'TagList', you should use
*	the function NextTagItem.  This will insure you respect any
*	chaining (TAG_MORE) that the list uses, and will skip any
*	TAG_IGNORE items that AllocateTagItems might use to stash
*	size and other information.
*
*    INPUTS
*	NumItems = the number of TagItem slots you want to allocate.
*
*    RESULT
*	TagList = the allocated chain of TagItem structures.  Will
*	    return NULL if unsuccessful.
*
*    BUGS
*
*    SEE ALSO
*	FreeTagItems, CloneTagItems
*
*****************************************************************************/

/*
 * assumes that CALLER will terminate the tag list
 */
struct TagItem	*	__asm
allocateTagItems( register __d0	ULONG	numitems )
{
    struct TagItem	*ti = NULL;	/* stupid lattice warning */

    if ( ti = (struct TagItem *)
	AllocMem( (numitems + 1) * sizeof *ti, MEMF_CLEAR) )
    {
	/* extra item so I know how long this allocated chunk is */
	ti->ti_Tag = TAG_IGNORE;
	ti->ti_Data = (numitems+1);
	ti++;
    }
    return ( ti );
}

/****** utility.library/RefreshTagItemClones *****************************
*
*    NAME
*	RefreshTagItemClones -- Rejuvenates a clone from the original.
*
*    SYNOPSIS
*	RefreshTagItemClones( CloneTagItems, OriginalTagItems )
*			         A0	     A1
*
*	struct TagItem *CloneTagItems;
*	struct TagItem *OriginalTagItems;
*
*    FUNCTION
*	If (and only if) the tag items 'CloneTagItems' were created
*	from 'OriginalTagItems' by CloneTagItems(), and if OriginalTagItems
*	has not been changed in any way, you can reset the clone list
*	to its original state by using this function.
*
*    INPUTS
*	CloneTagItems = return value from CloneTagItems( OriginalTagItems )
*	OriginalTagItems = a tag list that hasn't changed
*
*    RESULT
*	None
*
*    BUGS
*
*    SEE ALSO
*	CloneTagItems, AllocateTagItems, FreeTagItems
*
*****************************************************************************/

/*
 * assumes the contiguous structure of a clone that is
 * allocated by cloneTagItems()
 */
ULONG	__asm
refreshTagItemClones(register __a0	struct TagItem	*clone,
	register __a1	struct TagItem	*original )
{
    struct TagItem	*tstate;
    struct TagItem	*ti;

    tstate = original;

    while ( ti = nextTagItem( &tstate ) )
    {
	*clone = *ti;
	clone++;
    }
    clone->ti_Tag = TAG_DONE;
    return ( 0 );
}

/****** utility.library/CloneTagItems *************************************
*
*    NAME
*	CloneTagItems -- Copies a TagItem list.
*
*    SYNOPSIS
*	NewTagList = CloneTagItems( TagList )
*				      A0
*
*	struct TagItem *NewTagList;
*	struct TagItem *TagList;
*
*    FUNCTION
*	Copies the essential contents of a TagItem list.  Internally,
*	it uses AllocateTagItems so that you can use FreeTagItems.
*
*    INPUTS
*	TagList = TagItem list to clone.
*
*    RESULT
*	NewTagList = resultant copy.
*
*    BUGS
*
*    SEE ALSO
*	AllocateTagItems, FreeTagItems, RefreshTagItemClones
*
*****************************************************************************/

/* creates a copy of a tag list, collapsing TAG_MORE chaining,
 * and eliminating TAG_IGNORE.
 */
struct TagItem *	__asm
cloneTagItems(  register __a0 struct TagItem *tl )
{
    /* for iteration  of 'tl' */
    struct TagItem	*tstate;
    struct TagItem	*ti;
    int count;

    /* copy and indexing pointer */
    struct TagItem	*ticopy;

    tstate = tl;
    count = 1;		/* count one for the TAG_DONE I'll need to add */

    while ( ti = nextTagItem( &tstate ) )
    {
	count++;
    }

    if  ( ticopy = allocateTagItems( count ) )
    {
	refreshTagItemClones( ticopy, tl );
    }
    return ( ticopy );
}

#if UNUSED
/*
 * 'varargs' creation of tag list:
 * createTagItems( tag1, data1, tag2, data2, ..., TAG_DONE );
 */
struct TagItem	*
createTagItems( args )
int	args;
{
    return ( cloneTagItems( &args ) );
}
#endif


/****** utility.library/FreeTagItems *************************************
*
*    NAME
*	FreeTagItems	--  Frees allocated TagItem lists.
*
*    SYNOPSIS
*	FreeTagItems( TagList )
*			A0
*
*	struct TagItem	*TagList;
*
*    FUNCTION
*	Frees the memory of a TagItem list allocated either by
*	AllocateTagItems or CloneTagItems.
*
*    INPUTS
*	TagList = list to free.  Must be created by functions specified.
*	A value of NULL for 'TagList' is safe.
*
*    RESULT
*	None.
*
*    BUGS
*
*    SEE ALSO
*	AllocateTagItems, CloneTagItems
*
*****************************************************************************/

/*
 * assumes this was created by allocateTagItems() or
 * one of its callers.
 */
void	__asm
freeTagItems( register __a0 struct TagItem	*ti )
{
    if ( !ti ) return;

    ti--;	/* back to secret preamble */

    if ( ti->ti_Tag == TAG_IGNORE )
    {
	/* numitems stashed in ti_Data */
	FreeMem( ti, sizeof (struct TagItem) * ti->ti_Data );
    }
    D( else { kprintf( "FREEING BAD TAG ARRAY AT %lx\n", ti ); } )
}


#if UNUSED
struct TagItem	*map;
dumpTagItems( ti )
struct TagItem	*ti;
{
    struct TagItem	*tstate = ti;

    while ( ti = nextTagItem( &tstate ) )
    {
	kprintf("%8lx--%8lx\n", ti->ti_Tag, ti->ti_Data );
    }
}
#endif

/****** utility.library/TagInArray *************************************
*
*    NAME
*	TagInArray  -- Check if a Tag value appears in a Tag array
*
*    SYNOPSIS
*	BOOL	TagInArray( Tag, TagArray )
*		  	     D0	   A0
*
*	Tag		Tag;
*	Tag		*Tag;
*
*    FUNCTION
*	Perform a quick scan to see if a tag value appears in
*	an array terminated with TAG_END.  Returns TRUE if
*	the value is found.
*
*	The 'TagArray' must be terminated by TAG_END.  It should
*	NOT contain other system tag values, such as TAG_MORE
*	or TAG_IGNORE.
*
*	This is sort of a "one shot" version of FilterTagItems.
*
*    INPUTS
*	Tag = Tag value to search array for.
*	TagArray = a simple array terminated by TAG_END.
*
*    RESULT
*	Boolean success of search.
*
*    BUGS
*
*    SEE ALSO
*	FilterTagItems
*
*****************************************************************************/

/*
 * performs fast search for tag in array.
 * does not respect any system tag values except TAG_END
 */
BOOL __asm
tagInArray( register __d0 Tag	tag,
	    register __a0 Tag	*tagarray )
{
    while ( *tagarray != TAG_END )
    {
	if ( *tagarray++ == tag ) return ( TRUE );
    }
    return ( FALSE );
}

/****** utility.library/FilterTagItems *********************************
*
*    NAME
*	FilterTagItems - Remove selected items from a TagItem list.
*
*    SYNOPSIS
*	FilterTagList(TagList, TagArray, Logic)
*		  	A0	   A1	  D0
*
*	struct TagItem	*TagList;
*	Tag		*TagArray;
*	LONG		Logic;
*
*    FUNCTION
*	Removes TagItems from a TagItem list (by changing ti_Tag to
*	TAG_IGNORE) depending on whether its ti_Tag value is
*	found in an array of TagValues.
*
*	If the 'Logic' parameter is TAGFILTER_AND, then all items
*	not appearing in the list are excluded.
*
*	If 'Logic' is TAGFILTER_NOT, then items not found in the
*	array are preserved, and the ones in the array are cast out.
*
*    INPUTS
*	TagList = Input list of tag items which is to be filtered
*		by having selected items changed to TAG_IGNORE.
*	TagArray = an array of Tag values, terminated by TAG_END,
*		as specified in the notes on TagInArray().
*	Logic = specification whether items in TagArray are to
*		be included or excluded in the filtered result.
*
*    RESULT
*	Number of valid items left in resulting filtered list.
*
*    BUGS
*
*    SEE ALSO
*	TagInArray
*
*****************************************************************************/

ULONG	__asm
filterTagItems(	register __a0 struct TagItem	*taglist,
		register __a1 Tag		*tagarray,
		register __d0 int		logic )
{
    struct TagItem	*tistate = taglist;
    struct TagItem	*ti;
    ULONG		numitems = 0;

    if ( logic == TAGFILTER_AND )
    {
	/* include only the hits	*/
	while ( ti = nextTagItem( &tistate ) )
	{
	    if ( !tagInArray( ti->ti_Tag, tagarray ) )
		ti->ti_Tag = TAG_IGNORE;
	    else 
	    	numitems++;
	}
    }
    else	/* TAGFILTER_NOT */
    {
	/* include only the hits	*/
	while ( ti = nextTagItem( &tistate ) )
	{
	    if ( tagInArray( ti->ti_Tag, tagarray ) )
		ti->ti_Tag = TAG_IGNORE;
	    else 
	    	numitems++;
	}
    }
    return ( numitems );
}
