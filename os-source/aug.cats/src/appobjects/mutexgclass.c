/* mutexgclass.c
 * mutual exclusion class
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <libraries/apshattrs.h>
#include <libraries/appobjects.h>
#include <utility/tagitem.h>
#include <clib/macros.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <string.h>

extern struct Library *SysBase, *DOSBase;
extern struct Library *IntuitionBase, *GfxBase, *UtilityBase;

ULONG __saveds dispatchMutExG (Class * cl, Object * o, Msg msg);
ULONG setMutExGAttrs (Class * cl, Object * o, struct opSet * msg);
ULONG getMutExGAttrs (Class *cl, Object *o, struct opGet *msg);
VOID SetActiveObj (struct localObjData * lod, struct GadgetInfo * ginfo);
ULONG updateAttrChanges(Object *o, VOID *ginfo, ULONG flags, ULONG tag1, ... );
VOID SpreadAttr (Class *cl, Object *o, struct opSet *ops, Tag tag, ULONG data);

/* Class prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);

struct localObjData
{
    struct Gadget lod_Gadget;	/* Fake 'em out */
    struct MinList lod_List;	/* Members of exclusion group */
    ULONG lod_Flags;
};

#define	LODF_DISPOSE	0x0001

#define	MYCLASSID	"mutexgclass"
#define	SUPERCLASSID	"icclass"
#define	LSIZE		(sizeof(struct localObjData))

Class *initMutExGClass (VOID)
{
    ULONG hookEntry();
    Class *cl;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL, LSIZE, 0L))
    {
	/* Fill in callback hook */
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchMutExG;

	/* Make the class public */
	AddClass (cl);
    }

    return cl;
}

BOOL freeMutExGClass (Class * cl)
{
    return ((BOOL) FreeClass (cl));
}

ULONG __saveds dispatchMutExG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA(cl, o);
    Object *ostate, *member;
    ULONG retval = 0L;
    ULONG changes;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DSM (cl, o, msg))
	    {
		/* Get data from new object */
		lod = INST_DATA (cl, (Object *) retval);

		/* Initialize the list */
		NewList ((struct List *)&lod->lod_List);

		/* Nothing selected */
		lod->lod_Gadget.SpecialInfo = (APTR) -1;

		/* Set the attributes */
		setMutExGAttrs (cl, (Object *) retval, (struct opSet *) msg);
	    }
	    break;

	case OM_GET:
	    retval = getMutExGAttrs (cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    if (!(DoSuperMethod (cl, o, ICM_CHECKLOOP)))
	    {
		if (msg->MethodID == OM_SET)
		{
		    DSM (cl, o, msg);
		}

		if (changes = setMutExGAttrs (cl, o, (struct opSet *) msg))
		{
		    SetActiveObj(lod, ((struct opSet *) msg)->ops_GInfo);

		    if (changes & 1)
		    {
			notifyAttrChanges (o, ((struct opSet *)msg)->ops_GInfo,
			   FALSE,
			   GA_ID, (ULONG)lod->lod_Gadget.GadgetID,
			   GTMX_Active, (ULONG) lod->lod_Gadget.SpecialInfo,
			   TAG_END);
		    }
		}
	    }
	    break;

	case OM_ADDMEMBER:
	    /* Get a pointer to the member to add */
	    if (member = ((struct opAddMember *)msg)->opam_Object)
	    {
		/* Make sure things are set the way we need them. */
		SetAttrs (member,
			 GA_Immediate, TRUE,
			 GA_RelVerify, FALSE,
			 TAG_DONE);

		/* Add the member to the bottom of the list */
		DoMethod (member, OM_ADDTAIL, &lod->lod_List);
	    }
	    break;

	case OM_REMMEMBER:
	    /* Get a pointer to the member to remove */
	    member = ((struct opAddMember *)msg)->opam_Object;

	    /* Remove the member */
	    retval = (ULONG) DoMethod (member, OM_REMOVE, &lod->lod_List);
	    break;

	case OM_DISPOSE:
	    /* See if we should kill everyone */
	    if ((lod->lod_Flags & LODF_DISPOSE) || !(lod->lod_Gadget.UserData))
	    {
		/* Remove active members */
		ostate = (Object *) lod->lod_List.mlh_Head;
		while (member = NextObject (&ostate))
		{
		    DoMethod (member, OM_REMOVE);
		    DM (member, msg);
		}
	    }

	    /* Let the superclass remove the rest */
	    retval = DSM (cl, o, msg);
	    break;

	/* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DSM (cl, o, msg);
	    break;
    }

    return (retval);
}

ULONG setMutExGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG retval = 0;
    ULONG tidata;
    LONG newval;

    /* Initialize values */
    newval = (LONG) lod->lod_Gadget.SpecialInfo;

    /* Process tags */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case GA_UserData:
		lod->lod_Gadget.UserData = (APTR) tidata;
		break;

	    /* Gadget ID */
	    case GA_ID:
		lod->lod_Gadget.GadgetID = (USHORT) tidata;
		break;

	    case GA_Disabled:
		SpreadAttr (cl, o, msg, GA_Disabled, tidata);
		break;

	    /* See if they are changing the active one. */
	    case GTMX_Active:
		newval = tidata;
		break;
	}
    }

    /* See if we changed. */
    if (lod->lod_Gadget.SpecialInfo != (APTR) newval)
    {
	lod->lod_Gadget.SpecialInfo = (APTR) newval;
	retval = 1L;
    }

    return (retval);
}

ULONG getMutExGAttrs (Class *cl, Object *o, struct opGet *msg)
{
    struct localObjData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case GA_UserData:
	    *msg->opg_Storage = (ULONG) lod->lod_Gadget.UserData;
	    break;

	case GA_ID:
	    *msg->opg_Storage = (ULONG) lod->lod_Gadget.GadgetID;
	    break;

	case GTMX_Active:
	    *msg->opg_Storage = (ULONG) lod->lod_Gadget.SpecialInfo;
	    break;

	default:
	    return DSM (cl, o, msg);
    }
    return TRUE;
}

VOID SetActiveObj (struct localObjData * lod, struct GadgetInfo * ginfo)
{
    Object *ostate, *member;
    struct Gadget *g;

    /* Step through the list of gadgets */
    ostate = (Object *) lod->lod_List.mlh_Head;
    while (member = NextObject (&ostate))
    {
	g = (struct Gadget *) member;

	if (g->GadgetID == (USHORT) lod->lod_Gadget.SpecialInfo)
	{
	    if (!(g->Flags & SELECTED))
	    {
		updateAttrChanges(member,ginfo,NULL,GA_Selected,TRUE,TAG_END);
	    }
	}
	else
	{
	    if (g->Flags & SELECTED)
	    {
		updateAttrChanges(member,ginfo,NULL,GA_Selected,FALSE,TAG_END);
	    }
	}
    }
}

ULONG updateAttrChanges(Object *o, VOID *ginfo, ULONG flags, ULONG tag1, ... )
{
    return DoMethod( o, OM_UPDATE, &tag1, ginfo, flags );
}

VOID SpreadAttr (Class *cl, Object *o, struct opSet *ops, Tag tag, ULONG data)
{
    struct localObjData *lod = INST_DATA (cl, o);
    Object *ostate, *member;

    ostate = (Object *) lod->lod_List.mlh_Head;
    while (member = NextObject (&ostate))
    {
	updateAttrChanges(member, ops->ops_GInfo, NULL, tag, data, TAG_END);
    }
}

