/****** camdlists.lib/SelectCluster **********************************************
*
*   NAME
*       SelectCluster -- graphically select a MIDI cluster by name
*
*   SYNOPSIS
*		confirm = SelectCluster( listreq, buffer, maxlength, taglist );
*
*       LONG SelectCluster( void *, char *, LONG, Tag, ... );
*
*   FUNCTION
*       This function presents the user with a list of all active
*       clusters in camd.library. The user can click on an existing
*		cluster, or type in the name of a new cluster into the
*		supplied string gadget. "Select" and "Cancel" buttons are also
*		supplied.
*
*   INPUTS
*		listreq -- pointer to a previously allocated list requester
*		buffer -- where to put the name the user typed or selected.
*			In addition, this should be filled in with the name of
*			the existing cluster.
*		maxlength -- the length of the buffer.
*		tags -- the same tags values as AllocListRequest
*
*		These LISTREQ tags are passed through to the List Requester:
*
*			LISTREQ_Window		   		ASLFR_Window
*			LISTREQ_Screen	       		ASLFR_Screen
*			LISTREQ_UserData	   		ASLFR_UserData
*			LISTREQ_TitleText      		ASLFR_TitleText
*			LISTREQ_PositiveText   		ASLFR_PositiveText
*			LISTREQ_NegativeText   		ASLFR_NegativeText
*			LISTREQ_InitialLeftEdge		ASLFR_InitialLeftEdge
*			LISTREQ_InitialTopEdge 		ASLFR_InitialTopEdge
*			LISTREQ_InitialWidth  		ASLFR_InitialWidth
*			LISTREQ_InitialHeight  		ASLFR_InitialHeight
*			LISTREQ_HookFunc	     	ASLFR_HookFunc
*
*   RESULT
*      	confirm -- returns TRUE if the positive choice button was clicked,
*		or a list entry was double-clicked. Returns FALSE if the negative
*		choice button was clicked.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       AllocListRequest(), ListRequest(), FreeListRequest()
*
*****************************************************************************
*	Written by Talin
*/

#include <exec/types.h>
#include <exec/lists.h>
#include <midi/camd.h>
#include <clib/alib_protos.h>
#include <clib/camd_protos.h>
#include <pragmas/camd_pragmas.h>
#include <string.h>

#include "camdlists.h"

struct NameEntry {
	struct Node			node;
	char				name[ 32 ];
};

extern struct Library	*UtilityBase,
						*CamdBase;

LONG SelectCluster(
	struct ListRequester *lreq,					/* list requester				*/
	char				*buffer,				/* where to place cond. name	*/
	int					maxlen,					/* length of above buffer		*/
	Tag 				reqtags,				/* tags for list requester		*/
	...  				)
{	struct List			list;					/* list of item names			*/
	struct MidiCluster	*cl;					/* cluster structure			*/
	struct NameEntry 	*ne;					/* holds the item name			*/
	LONG				result = FALSE;			/* result of this function		*/
	APTR				lock;

	NewList( &list );							/* initialize the list			*/
												/* walk cluster list			*/
	lock = LockCAMD( CD_Linkages );
	for (cl = NextCluster( NULL ); cl; cl = NextCluster ( cl ) )
	{
												/* allocate node to hold name	*/
		if ( ne = AllocMem( sizeof *ne, MEMF_CLEAR ))
		{	struct Node			*search;

			strncpy( ne->name, cl->mcl_Node.ln_Name, sizeof ne->name );
			ne->node.ln_Name = ne->name;

			for (	search=list.lh_Head ;		/* insert them alphabetically	*/
					search->ln_Succ ;
					search=search->ln_Succ )
			{	if (Stricmp( search->ln_Name, ne->node.ln_Name ) >= 0) break;
			}
			Insert( &list, &ne->node, search->ln_Pred );
		}
		else break;
	}
	UnlockCAMD( lock );

	if (ListRequest(lreq,
					LISTREQ_Labels,		&list,
					LISTREQ_Buffer, 	buffer,
					LISTREQ_BufferSize, maxlen,
					TAG_MORE, 			&reqtags,
					TAG_END ))
	{	result = TRUE;
	}

	while (ne = (struct NameEntry *)RemHead( &list )) FreeMem( ne, sizeof *ne );

	return result;
}
