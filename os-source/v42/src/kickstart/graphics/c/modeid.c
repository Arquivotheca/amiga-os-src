/******************************************************************************
*
*       $Id: modeid.c,v 42.0 93/06/16 11:16:32 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <graphics/view.h>
#include <graphics/videocontrol.h>
#include <graphics/displayinfo.h>
#include "/d/d.protos"
#include "c.protos"

struct TagItem template[] =
{
    { VTAG_NORMAL_DISP_GET, NULL }, 
    { VTAG_VPMODEID_GET, NULL }, 
    { VTAG_END_CM, NULL }             
};

ULONG __asm GetVPModeID(register __a0 struct ViewPort *vp)
{
    struct TagItem commands[ (sizeof( template )/ sizeof( struct TagItem )) ];
    ULONG modeID = INVALID_ID;
    struct QueryHeader query;
    int i;

    for( i = 0; i < (sizeof( template )/ sizeof( struct TagItem )); i++ )
    {
		*(commands+i) = *(template+i);
    }

    if(!(videocontrol( vp->ColorMap, commands )))
    {
	    if((modeID = commands[1].ti_Data) == INVALID_ID)
		{
			DisplayInfoHandle handle = (DisplayInfoHandle)commands[0].ti_Data;
	
			if(GetDisplayInfoData(handle,(UBYTE *) &query,sizeof(query),DTAG_DISP,modeID))
			{
				modeID = query.DisplayID; /* determine modeID */
			}
		}
    }

    return( modeID );
}

ULONG __asm ModeNotAvailable(register __d0 ULONG modeID)
{
    struct DisplayInfo dinfo;

    if ( ((ULONG)GetDisplayInfoData(NULL,(UBYTE *) &dinfo,sizeof(dinfo),DTAG_DISP,modeID)) >
       ( (ULONG) (&((struct DisplayInfo *)0)->PropertyFlags) )  )

	    return((ULONG) dinfo.NotAvailable); /* determine availability */
	else
	    return( INVALID_ID );
}
