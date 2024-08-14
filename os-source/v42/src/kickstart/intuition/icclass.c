/*** icclass.c *******************************************************
 *
 * icclass.c -- interconnection class
 *
 *  $Id: icclass.c,v 40.0 94/02/15 18:07:56 davidj Exp $
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

/*****************************************************************************/

#define D(x)	;
#define DL(x)	;
#define DIC(x)	;

/*****************************************************************************/

struct localData
{
    Object		*lod_Target;
    struct TagItem	*lod_MapList;	/* shared pointer: not a clone	*/
    ULONG		 lod_LoopCount;	/* padded to longword */
};

/*****************************************************************************/

static ULONG omGet (Class *cl, Object *o, struct opGet *msg)
{
    switch (msg->opg_AttrID)
    {
	/* Don't allow anyone to "get" these attributes */
	case ICA_TARGET:
	case ICA_MAP:
	    return 0;

	default:
	    return SSM (cl, o, msg);
    }
}

/*****************************************************************************/

static ULONG omSet (Class *cl, Object *o, struct opSet *msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *ti;
    struct localData *lod = INST_DATA (cl, o);

    /* new mapping list */
    if (ti = FindTagItem (ICA_MAP, tags))
    {
	/* don't clone	*/
	lod->lod_MapList = (struct TagItem *) ti->ti_Data;
    }

    /* new target */
    if (ti = FindTagItem (ICA_TARGET, tags))
    {
	lod->lod_Target = (Object *) ti->ti_Data;
	D (printf ("ic (sub-)class setting primary target: %lx\n", lod->lod_Target));
    }

    return (0);
}

/*****************************************************************************/

static ULONG Dispatcher (Class *cl, Object *o, Msg msg)
{
    struct localData *lod = INST_DATA (cl, o);
    Object *newobj;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) SSM (cl, o, msg))
		omSet (cl, newobj, (struct opSet *) msg);
	    return ((ULONG) newobj);

	case ICM_SETLOOP:
	    lod->lod_LoopCount++;
	    DL (printf ("icclass SETLOOP: %ld\n", lod->lod_LoopCount));
	    break;

	case ICM_CLEARLOOP:
	    lod->lod_LoopCount--;
	    DL (printf ("icclass CLEARLOOP : %ld\n", lod->lod_LoopCount));
	    break;

	case ICM_CHECKLOOP:
	    DL (printf ("icclass CHECKLOOP ====: %ld\n", lod->lod_LoopCount));
	    return (lod->lod_LoopCount);
	    break;

	case OM_SET:
	    return (SSM (cl, o, msg) + omSet (cl, o, (struct opSet *) msg));

	case OM_GET:
	    return omGet (cl, o, (struct opGet *) msg);

	case OM_NOTIFY:	/* some subclass wants action	*/
	case OM_UPDATE:	/* I map and forward updates	*/
	    /* ZZZ: should this be sent to true class or
	 * coerced to me?  If latter, I may as well just
	 * bump the damn counter myself right here.
	 */

#if 1				/* this is the faster, non-overrideable version */
	    if (lod->lod_Target && (lod->lod_LoopCount == 0))
	    {
		lod->lod_LoopCount++;

		DIC (printf ("notify primary ic target at %lx\n",
			     lod->lod_Target));
		/* convert input attlist in situ to mapped tags */
		MapTags (((struct opUpdate *) msg)->opu_AttrList,
			 lod->lod_MapList, MAP_KEEP_NOT_FOUND);

		msg->MethodID = OM_UPDATE;

		/* and forward the whole shebang to target */

		/* special target value of ~0 means send to window */
		if ((ULONG) lod->lod_Target == ~0)
		    sendNotifyIDCMP ((struct opUpdate *) msg);
		else
		    SM (lod->lod_Target, msg);

		lod->lod_LoopCount--;
	    }
	    DL (
		   else
		   printf ("icclass: no notify: target %lx, loop %lx\n",
			   lod->lod_Target, lod->lod_LoopCount));

#else				/* slower, but more oopsi	*/
	    if (lod->lod_Target && SendMessage (o, ICM_CHECKLOOP))
	    {
		SendMessage (o, ICM_SETLOOP);

		/* convert input attlist in situ to mapped tags */
		MapTags (((struct opSet *) msg)->ops_AttrList,
			 lod->lod_MapList, MAP_KEEP_NOT_FOUND);

		msg->MethodID = OM_UPDATE;

		/* and forward the whole shebang to target */
		SM (lod->lod_Target, msg);
		SendMessage (o, ICM_CLEARLOOP);
	    }
#endif
	    break;

	case OM_DISPOSE:
	    /* don't free tag list or target	*/
	    SSM (cl, o, msg);
	    break;

#if 0
	case OM_ADDTAIL:	/* I get stuck in lists a lot	*/
#endif
	default:		/* let the superclass handle it */
	    return SSM (cl, o, msg);
    }
    return (1);
}

/*****************************************************************************/

Class *initICClass (void)
{
    extern UBYTE *RootClassName;
    extern UBYTE *ICClassName;

    return (makePublicClass (ICClassName, RootClassName, sizeof (struct localData), Dispatcher));
}
