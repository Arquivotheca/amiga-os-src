/* lgroupgclass.c -- gadget group class
 *
 */

#include "appobjectsbase.h"

#define	DB(x)	;

VOID NewList (struct MinList *);

/* lgroupgclass.c */
Class *initLGroupGClass (VOID);
ULONG freeLGroupGClass (Class * cl);
static ULONG __saveds dispatchLGroupG (Class * cl, Object * o, Msg msg);
static VOID translateCGPInput (Class *cl, Object * o, Object * g, struct gpInput * msg);
static ULONG propagateHit (Object * o, struct localObjData * lod, struct gpHitTest * msg);
static ULONG setLGroupGAttrs (Class * cl, Object * o, struct opSet * msg);
static ULONG getLGroupGAttrs (Class * cl, Object * o, struct opGet * msg);
VOID gadgetBox (struct Gadget *, struct IBox *, struct IBox *, struct IBox *, ULONG);

struct localObjData
{
    struct IBox lod_Domain;	/* Domain of the gadget */
    ULONG lod_Flags;		/* See defines below (and appobjectsbase) */
    struct MinList lod_AList;	/* Active gadget list */
    Object *lod_Active;		/* Active member, if any */
    Object *lod_Master;		/* Gadget to activate when no active member */
};

/* transparent base class */
#define G(o)	((struct Gadget *)(o))

#define	MYCLASSID	"lgroupgclass"
#define	SUPERCLASSID	GADGETCLASS
#define	INSTANCESIZE	sizeof(struct localObjData)

Class *initLGroupGClass (VOID)
{
    ULONG hookEntry ();
    Class *cl;

    if (cl = MakeClass (MYCLASSID, SUPERCLASSID, NULL,INSTANCESIZE, 0))
    {
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchLGroupG;
	AddClass (cl);
    }
    return (cl);
}

ULONG freeLGroupGClass (Class * cl)
{
    return ((ULONG) FreeClass (cl));
}

static ULONG __saveds dispatchLGroupG (Class * cl, Object * o, Msg msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    ULONG retval = 1L;
    Object *member;
    Object *ostate;
    Object *tmpobj;

    DB (kprintf ("dispatchLGroupG : %ld\n", msg->MethodID));
    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (tmpobj = (Object *) DSM (cl, o, msg))
	    {
		lod = INST_DATA (cl, tmpobj);
		NewList (&lod->lod_AList);
		setLGroupGAttrs (cl, tmpobj, (struct opSet *) msg);
	    }
	    retval = (ULONG) tmpobj;
	    break;

	case OM_SET:
	    retval = DSM (cl, o, msg);
	    retval += setLGroupGAttrs (cl, o, (struct opSet *) msg);
	    break;

	case GM_RENDER:
	    ostate = (Object *) lod->lod_AList.mlh_Head;
	    while (member = NextObject (&ostate))
	    {
		DM (member, msg);
	    }
	    retval = 1L;
	    break;

	case GM_HITTEST:
	    retval = propagateHit (o, lod, (struct gpHitTest *) msg);
	    break;

	case GM_GOACTIVE:
            if (!(tmpobj = ((lod->lod_Active) ? lod->lod_Active : lod->lod_Master)))
	    {
                retval = GMR_REUSE;
	    }
            else
            {
                lod->lod_Active = tmpobj;
                translateCGPInput (cl, o, tmpobj, (struct gpInput *) msg);
                if ((retval = DM (tmpobj, msg)) == GMR_MEACTIVE)
                    G(tmpobj)->Activation |= ACTIVEGADGET;
            }
	    break;

	case GM_HANDLEINPUT:
	    if (lod->lod_Active)
	    {
		translateCGPInput (cl, o, lod->lod_Active, (struct gpInput *) msg);
		retval = (ULONG) DM (lod->lod_Active, msg);
	    }
	    break;

	case GM_GOINACTIVE:
	    if (lod->lod_Active && (G(lod->lod_Active)->Activation & ACTIVEGADGET))
	    {
		DM (lod->lod_Active, msg);
		G(lod->lod_Active)->Activation &= ~ACTIVEGADGET;
	    }
	    lod->lod_Active = NULL;
	    break;

	case OM_ADDMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    DoMethod (member, OM_ADDTAIL, &lod->lod_AList);
	    break;

	case OM_REMMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    retval = (ULONG) DoMethod (member, OM_REMOVE, &lod->lod_AList);
	    break;

	case OM_DISPOSE:
	    ostate = (Object *) lod->lod_AList.mlh_Head;
	    while (member = NextObject (&ostate))
	    {
		DoMethod (member, OM_REMOVE);
		DM (member, msg);
	    }
	    retval = DSM (cl, o, msg);
	    break;

	default:
	    return ((ULONG) DSM (cl, o, msg));
    }
    DB (kprintf ("dispatchLGroupG exit\n"));
    return (retval);
}

static VOID translateCGPInput (Class *cl, Object * o1, Object * o2, struct gpInput * msg)
{
    struct localObjData *lod = INST_DATA(cl, o1);

    msg->gpi_Mouse.X -= (G(o2)->LeftEdge - lod->lod_Domain.Left);
    msg->gpi_Mouse.Y -= (G(o2)->TopEdge - lod->lod_Domain.Top);
}

static ULONG propagateHit (Object * o, struct localObjData * lod, struct gpHitTest * msg)
{
    struct IBox mouse;
    struct IBox gbox;
    Object *member;
    Object *ostate;
    int origx;
    int origy;

    gadgetBox (G(o), &(msg->gpht_GInfo->gi_Domain), &(lod->lod_Domain), NULL, NULL);
    origx = msg->gpht_Mouse.X + lod->lod_Domain.Left;
    origy = msg->gpht_Mouse.Y + lod->lod_Domain.Top;
    ostate = (Object *) lod->lod_AList.mlh_Head;
    while (member = NextObject (&ostate))
    {
	gadgetBox (G(member), &(msg->gpht_GInfo->gi_Domain), &gbox, NULL, NULL);
	mouse.Left = msg->gpht_Mouse.X = (origx - gbox.Left);
	mouse.Top = msg->gpht_Mouse.Y = (origy - gbox.Top);
	gbox.Left = gbox.Top = 0;
	if (PointInBox (&mouse, &gbox) && DM (member, msg))
	{
	    if (G (member)->Flags & GADGDISABLED)
	    {
		return (0);
	    }
	    lod->lod_Active = member;
	    return (GMR_GADGETHIT);
	}
    }
    return (0L);
}

static ULONG setLGroupGAttrs (Class * cl, Object * o, struct opSet * msg)
{
    struct localObjData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tag;
    ULONG tidata;
    ULONG retval = 0L;
    int deltaleft = 0;
    int deltatop = 0;
    Object *member;
    Object *ostate;

    while (tag = NextTagItem (&tags))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case GA_Left:
		deltaleft = ((int) tidata) - G(o)->LeftEdge;
		break;

	    case GA_Top:
		deltatop = ((int) tidata) - G(o)->TopEdge;
		break;

	    case CGTA_Master:
		lod->lod_Master = (Object *) tidata;
		break;
	}
    }

    if (deltaleft || deltatop)
    {
	ostate = (Object *) lod->lod_AList.mlh_Head;
	while (member = NextObject (&ostate))
	{
	    SetGadgetAttrs (G (member),
			    msg->ops_GInfo->gi_Window,
			    msg->ops_GInfo->gi_Requester,
			    GA_LEFT, (G (member)->LeftEdge + deltaleft),
			    GA_TOP, (G (member)->TopEdge + deltatop),
			    TAG_END);
	}
	G (o)->LeftEdge += deltaleft;
	G (o)->TopEdge += deltatop;
    }
    return (retval);
}
