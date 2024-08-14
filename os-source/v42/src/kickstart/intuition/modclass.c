/*** modclass.c *******************************************************
 *
 * modclass.c -- base model class
 *	a model is at root just an update broadcaster
 *
 *  $Id: modclass.c,v 40.0 94/02/15 17:33:58 davidj Exp $
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

#define DIC(x)	;

/*****************************************************************************/

/* A Model is just an IC with an extra list of IC's
 * to broadcast things to.
 */
struct ModData
{
    struct MinList md_Dependents;
};

/*****************************************************************************/

static void notifyMod (Class *cl, Object *o, struct opUpdate *msg, struct ModData *md)
{
    struct TagItem *origattrs;
    Object *ostate;
    Object *ic;

    DIC (printf (">>>\nnodifyMod model at %lx\n", o));

    /* propagate notification message to dependents.
     * supercede this with model-specific behaviour.
     */
    if (SendSuperMessage (cl, o, ICM_CHECKLOOP) != 0)
	return;

    /* This is the place where these messages are broadcast.
     * Since the receivers are allowed to change the
     * attribute list in the message, to broadcast I
     * need to send a clones.  We reuse the message
     * but refresh the cloned attribute list.
     */

    /* first we broadcast the message to our dependents,
     * (probably controls), then pass it out to
     * our main target.
     */
    SendSuperMessage (cl, o, ICM_SETLOOP);
    origattrs = msg->opu_AttrList;
    msg->opu_AttrList = CloneTagItems (origattrs);

    msg->MethodID = OM_UPDATE;	/* going external	*/

    ostate = (Object *) md->md_Dependents.mlh_Head;
    while (ic = NextObject (&ostate))
    {
	DIC (printf ("update member at %lx --", ic));
	SM (ic, msg);
	RefreshTagItemClones (msg->opu_AttrList, origattrs);
    }
    SendSuperMessage (cl, o, ICM_CLEARLOOP);

    DIC (printf ("model at %lx: have updated my members, msg %lx atts %lx\n",
		 o, msg, msg->opu_AttrList));
    DIC (Debug ());

    /* let the IC superclass send to the default target	*/
    SSM (cl, o, msg);
    DIC (printf ("model at %lx: have passed to super.\n<<<\n", o));
    FreeTagItems (msg->opu_AttrList);

    /* restore, for caller */
    msg->opu_AttrList = origattrs;
}

/*****************************************************************************/

static ULONG modDispatch (Class *cl, Object *o, Msg msg)
{
    Object *newobj;
    struct ModData *md;

    Object *ic;
    Object *ostate;

    md = INST_DATA (cl, o);

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) SSM (cl, o, msg))
	    {
		md = INST_DATA (cl, newobj);	/* new object */
		NewList ((struct List *) &md->md_Dependents);

		/* no attributes */
	    }

	    return ((ULONG) newobj);

	case OM_NOTIFY:
	    DIC (printf ("model at %lx receives OM_NOTIFY!\n"));
	case OM_UPDATE:
	    /* would SendMessage be better?  Certainly not faster! */
	    notifyMod (cl, o, (struct opUpdate *) msg, md);
	    break;

	case OM_ADDMEMBER:
	    /* add an IC to the dependents list */
	    SendMessage (((struct opAddMember *) msg)->opam_Object,
			 OM_ADDTAIL, &md->md_Dependents);
	    break;

	case OM_REMMEMBER:
	    SendMessage (((struct opAddMember *) msg)->opam_Object,
			 OM_REMOVE, &md->md_Dependents);
	    break;

	case OM_DISPOSE:
	    /* dispose of all IC's in my list */
	    ostate = (Object *) md->md_Dependents.mlh_Head;
	    while (ic = NextObject (&ostate))
	    {
		SendMessage (ic, OM_REMOVE);
		SendMessage (ic, OM_DISPOSE);
	    }
	    SSM (cl, o, msg);
	    break;

#if 0
	case OM_SET:
#endif
	default:		/* let the superclass handle it */
	    return ((ULONG) SSM (cl, o, msg));
    }
    return (1);
}

/*****************************************************************************/

Class *initModelClass (void)
{
    extern UBYTE *ModelClassName;
    extern UBYTE *ICClassName;

    return (makePublicClass (ModelClassName, ICClassName,
			     sizeof (struct ModData), modDispatch));
}
