/*
 * NoInterleaveWB - Force SA_Interleaved off for the Workbench screen
 * by SetFunctioning OpenScreenTagList().
 *
 * lc -d -L+lib:debug.lib -b0 -v NoInterleaveWB
 */

#include <intuition/intuition.h>
#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/utility_pragmas.h>

extern UWORD LVOOpenScreenTagList;

struct Screen * (* __asm oldOpenScreenTagList)(
	register __a0 struct NewScreen *ns,
	register __a1 struct TagItem *taglist,
	register __a6 struct Library *IntuitionBase );

struct Screen * __asm newOpenScreenTagList(
	register __a0 struct NewScreen *ns,
	register __a1 struct TagItem *taglist,
	register __a6 struct Library *ibase );

struct Library *UtilityBase;
struct Library *IntuitionBase;


main()
{
    if ( IntuitionBase = OpenLibrary("intuition.library", 39) )
    {
	if ( UtilityBase = OpenLibrary("utility.library", 37) )
	{
	    oldOpenScreenTagList = SetFunction( IntuitionBase,
		&LVOOpenScreenTagList, (APTR)newOpenScreenTagList );
	    Wait(0);
	}
	CloseLibrary( IntuitionBase );
    }
}



struct Screen * __asm newOpenScreenTagList(
	register __a0 struct NewScreen *ns,
	register __a1 struct TagItem *taglist,
	register __a6 struct Library *ibase )
{
    struct TagItem newtags[] =
    {
	SA_Interleaved, FALSE,
	TAG_MORE, 0,
    };

    if ( GetTagData( SA_Type, NULL, taglist ) == WBENCHSCREEN )
    {
	/* Add the SA_Interleaved,TRUE tagitem, and use the
	 * rest of the old tag-list.
	 */
	newtags[1].ti_Data = (ULONG) taglist;
	taglist = newtags;
    }
    return( (*oldOpenScreenTagList)( ns, taglist, ibase ) );
}
