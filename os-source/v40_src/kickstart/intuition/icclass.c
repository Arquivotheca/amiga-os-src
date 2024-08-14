
/*** icclass.c *******************************************************
 *
 * icclass.c -- interconnection class
 *
 *  $Id: icclass.c,v 38.1 92/02/07 11:05:48 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "icclass.h"

#define D(x)	;
#define DL(x)	;
#define DIC(x)	;

struct ICData	{
    Object		*icd_Target;
    struct TagItem	*icd_MapList;	/* shared pointer: not a clone	*/
    ULONG		icd_LoopCount;	/* padded to longword		*/
};

Class	*
initICClass()
{
    Class	*makePublicClass();
    ULONG 	icDispatch();
    extern UBYTE	*ICClassName;
    extern UBYTE	*RootClassName;

    return (  makePublicClass( ICClassName, RootClassName, 
    	sizeof ( struct ICData ), icDispatch));
}

ULONG 
icDispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct ICData	*icd;

    icd = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    case ICM_SETLOOP:
	icd->icd_LoopCount++;
	DL( printf("icclass SETLOOP: %ld\n", icd->icd_LoopCount ));
	break;

    case ICM_CLEARLOOP:
	icd->icd_LoopCount--;
	DL( printf("icclass CLEARLOOP : %ld\n", icd->icd_LoopCount ));
	break;

    case ICM_CHECKLOOP:
	DL( printf("icclass CHECKLOOP ====: %ld\n", icd->icd_LoopCount ));
	return ( icd->icd_LoopCount );
	break;

    case OM_NEW:
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    setICAttrs( cl, newobj, msg );
	}
	return ( (ULONG) newobj );

    case OM_SET:	
	return ( (ULONG) 
		SSM( cl, o, msg ) +
		setICAttrs( cl, o, msg ) );

    case OM_GET:	
	return ( (ULONG) getICAttrs( cl, o, msg, icd ) );

    case OM_NOTIFY:	/* some subclass wants action	*/
    case OM_UPDATE:	/* I map and forward updates	*/
	/* ZZZ: should this be sent to true class or
	 * coerced to me?  If latter, I may as well just
	 * bump the damn counter myself right here.
	 */

#if 1	/* this is the faster, non-overrideable version */
	if ( icd->icd_Target && (icd->icd_LoopCount == 0) )
	{
	    icd->icd_LoopCount++;

	    DIC( printf("notify primary ic target at %lx\n",
		icd->icd_Target ));
	    /* convert input attlist in situ to mapped tags */
	    MapTags( ((struct opUpdate *)msg)->opu_AttrList,
		icd->icd_MapList, MAP_KEEP_NOT_FOUND );

	    msg->MethodID = OM_UPDATE;

	    /* and forward the whole shebang to target */

	    /* special target value of ~0 means send to window */
	    if ( (ULONG) icd->icd_Target == ~0 )
		sendNotifyIDCMP( msg );
	    else
		SM( icd->icd_Target, msg );

	    icd->icd_LoopCount--;
	}
	DL( else printf("icclass: no notify: target %lx, loop %lx\n",
		icd->icd_Target, icd->icd_LoopCount ) );

#else	/* slower, but more oopsi	*/
	if ( icd->icd_Target && SendMessage( o, ICM_CHECKLOOP ) )
	{
	    SendMessage( o, ICM_SETLOOP );

	    /* convert input attlist in situ to mapped tags */
	    MapTags( ((struct opSet *)msg)->ops_AttrList,
		icd->icd_MapList, MAP_KEEP_NOT_FOUND );

	    msg->MethodID = OM_UPDATE;

	    /* and forward the whole shebang to target */
	    SM( icd->icd_Target, msg );
	    SendMessage( o, ICM_CLEARLOOP );
	}
#endif
	break;

    case OM_DISPOSE:
	/* don't free tag list or target	*/
	SSM( cl, o, msg );
	break;

#if 0
    case OM_ADDTAIL:	/* I get stuck in lists a lot	*/
#endif
    default:		/* let the superclass handle it */
	return ( (ULONG) SSM( cl, o, msg ) );
    }
    return ( 1 );
}

getICAttrs( cl, o, msg, icd )
Class		*cl;
Object		*o;
struct opGet	*msg;
struct ICData	*icd;
{
    switch ( msg->opg_AttrID )
    {
    case ICA_TARGET:
    case ICA_MAP:
	break;		/* these are not "get access"	*/
    default:
	return ( SSM( cl, o, msg ) );
    }
    return ( 0 );
}

setICAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*FindTagItem();
    struct TagItem	*CloneTagItems();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*ti;
    struct ICData	*icd = INST_DATA( cl, o );

    /* new mapping list */
    if ( ti = FindTagItem( ICA_MAP, tags ) )
    {
	/* don't clone	*/
	icd->icd_MapList = (struct TagItem *) ti->ti_Data;
    }

    /* new target */
    if ( ti = FindTagItem( ICA_TARGET, tags ) )
    {
	icd->icd_Target = (Object *) ti->ti_Data;
	D( printf("ic (sub-)class setting primary target: %lx\n",
		icd->icd_Target ) );
    }

    return ( 0 );	/* or whatever */
}
