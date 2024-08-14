/* tagdebug.c -- dumpTagList() routine  :ts=8	*/


#include "sysall.h"

#ifdef printf
#undef printf
#endif

#define printf kprintf

dumpTagList( str, tags )
UBYTE		*str;
struct TagItem	*tags;
{
    struct TagItem	*ti;
    struct TagItem	*NextTagItem();

    printf("\n");
    if ( str ) printf( "%s\n", str );
    printf("tags at %lx, prevtag %08lx - %08lx\n",
    	tags, tags[-1].ti_Tag, tags[-1].ti_Data );


    while ( ti = NextTagItem( &tags ) )
    {
	printf("%08lx - %08lx\n", ti->ti_Tag, ti->ti_Data );
    }
}
