/* testclient.c -- test and example for utility.library	*/

#ifdef UTILITYINTERNAL
#include "tagitem.h"
#include "utility_pragmas.h"
#include "utility_protos.h"
#else
#include <utility/tagitem.h>
#include <utility/utility_pragmas.h>	/* where do we keep this file? */
#include <utility/utility_protos.h>	/* where do we keep this file? */
#endif

void	*UtilityBase = NULL;
#include <proto/exec.h>

enum mytags { tag1 = TAG_USER+1, tag2, tag3, tag4, tag5 };

struct TagItem	taglist1[] = {
    { tag1,  0xdead },
    { TAG_IGNORE,  0 },
    { tag2,  0xface },
    { TAG_SKIP,  1 },
    { tag5,  0x0666 },
    { tag3,  0xbabe },
    { tag4,  0xcaca },
    { TAG_DONE },
};

struct TagItem	boolmap[] = {
    { tag1,  0x0001 },
    { tag2,  0x0002 },
    { tag3,  0x0004 },
    { tag4,  0x0005 },
    { TAG_DONE }
};

struct TagItem	boolexample[] = {
    { tag1,  TRUE },
    { tag2,  FALSE },
    { tag3,  TRUE },
    { TAG_DONE }
};

/* ********************************************	*/
/* my test program				*/
/* ********************************************	*/

main()
{
    struct TagItem	*tagitem;
    ULONG		boolflags;
    ULONG		tagdata;

    UtilityBase = OpenLibrary( "utility.library", 36 );
    if ( ! UtilityBase )
    {
    	puts ( "can't open utility.library." );
	exit ( 20 );
    }
    else
    {
    	printf( "utility.library open OK at %lx\n", UtilityBase );
    }

    /* this function demonstrates NextTagItem */
    dumpTagList( "example tag list", taglist1 );

    /* scan a tag list for an item with a particular tag,
     * respecting all the control tags.
     * try two Finds, one miss, one hit
     */
    if ( tagitem = FindTagItem( tag5, taglist1 ) )
    {
    	dumpTagItem( "found item:", tagitem );
    }
    else puts( "tag5 not found." );

    if ( tagitem = FindTagItem( tag4, taglist1 ) )
    {
    	dumpTagItem( "found item:", tagitem );
    }
    else puts( "tag4 not found." );

    /* if you plan on using a default if the tag value is
     * not found, you can use this function.
     */
    tagdata = GetTagData( tag4, 0x99, taglist1 );
    printf( "GetTagData returned %lx (default 0x99).\n", tagdata );
    tagdata = GetTagData( tag5, 0x99, taglist1 );

    /* test out the boolean tags accumulation */
    boolflags = PackBoolTags( 0x80000, boolexample, boolmap );
    printf( "boolean flags accumulated: %lx\n", boolflags );

    CloseLibrary( UtilityBase );
}

dumpTagList( str, taglist )
UBYTE		*str;
struct TagItem	*taglist;
{
    struct TagItem	*tagitem;

    if ( str ) puts( str );

    while ( tagitem = NextTagItem( &taglist ) )
    {
    	dumpTagItem( "\t", tagitem );
    }
}

dumpTagItem( str, tagitem )
UBYTE		*str;
struct TagItem	*tagitem;
{
    if ( str ) printf( "%s\t", str );
    if ( tagitem )
    	printf("{ 0x%8x, 0x%8x }\n", tagitem->ti_Tag, tagitem->ti_Data );
    else
	puts( "NULL TAGITEM PASSED TO dumpTagItem!!!" );
}
